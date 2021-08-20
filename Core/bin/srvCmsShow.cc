#include "TRandom3.h"
#include "TSystem.h"
#include "TServerSocket.h"
#include "TEnv.h"
#include "TROOT.h"
#include "TFile.h"
#include "TApplication.h"

#include "ROOT/REveManager.hxx"
#include "ROOT/RWebWindow.hxx"

#include <cstdio>
#include <string>
#include <regex>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <signal.h>


#include <boost/program_options.hpp>
#include "FireworksWeb/Core/interface/FW2Main.h"

static int FIREWORKS_SERVICE_PORT = 6666;

static const char* const kPortCommandOpt = "port";
static const char* const kInputFilesCommandOpt = "input-files,i";
static const char* const kInputFilesOpt        = "input-files";
static const char* const kHelpOpt        = "help";
static const char* const kHelpCommandOpt = "help,h";

//=============================================================================
// Message Queue stuff
//=============================================================================

int         global_msgq_id;
long        global_server_id = -1; // set for children after fork from N_total_children

struct ChildStatus
{
   int nConn;
   std::time_t mirTime;
   std::time_t disconnectTime;
   pid_t pid;

   float getIdleScore()
   {
      float score;
      std::time_t now = std::time(nullptr);
      if (nConn == 0)
      {
         score = 100 * difftime(now, disconnectTime);
      }
      else
      {
         // if Nconnection == -1, means noone has connected yet, mirTime is the server creation time
         // at the moment both cases are treated the same
         score = difftime(now, mirTime);
      }
      return score;
   }
};

struct fw_msgbuf {
   long mtype;       // message type, must be > 
   ChildStatus mtext;  // to be replaced by struct
};

void msgq_receiver_thread_foo()
{
   while ( true )
   {
      struct fw_msgbuf msg;

      if (msgrcv(global_msgq_id, (void *) &msg, sizeof(msg.mtext), 0, 0) == -1) {
         if (errno == EIDRM) {
            printf("message queue listener thread terminating on queue removal\n");
            break;
         }
         perror("msgrcv");
      } else {
         printf("message received from id %lu, status: (N=%d, MIR=%lu, Dissconn=%lu)\n", 
         msg.mtype, 
         msg.mtext.nConn, 
         msg.mtext.mirTime, msg.mtext.disconnectTime);
      }
   }
}

void msgq_test_send(int id, ROOT::Experimental::REveManager::ClientStatus& cs)
{
   struct fw_msgbuf msg;
   msg.mtype = id;
   msg.mtext.nConn = cs.fNConnections;
   msg.mtext.mirTime = cs.fMIRTime;
   msg.mtext.disconnectTime = cs.fDisconnectTime;


   if (msgsnd(global_msgq_id, (void *) &msg, sizeof(msg.mtext), IPC_NOWAIT) == -1) {
      perror("msgsnd error");
      return;
   }
   printf("sent status: N conn = %d\n", msg.mtext.nConn);
}

struct StatReportTimer : public TTimer
{
   bool Notify() override
   {
      using namespace ROOT::Experimental;
      REveManager::ClientStatus cs;
      gEve->GetClientStatus(cs);
      msgq_test_send(global_server_id, cs);
      Reset();
      return true;
   }
};

//=============================================================================
// Signal and child process handling
//=============================================================================

std::map<pid_t, int> children;

static void child_handler(int sig)
{
    pid_t pid;
    int status;

    printf("Got SigCHLD ... entering waitpid loop.\n");

    while((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
       auto i = children.find(pid);
       if (i != children.end())
       {
          printf("Child pid=%d id=%d died ... cleaning up.\n", i->first, i->second);
          children.erase(i);
       }
       else
       {
          printf("Got SigCHLD for pid=%d, not in map, hmmh.\n", pid);
       }
    }
}

bool ACCEPT_NEW = true;

static void int_handler(int sig)
{
    printf("Got SigINT/TERM, exiting main loop, will reap children there.\n");
    ACCEPT_NEW = false;
}

//=============================================================================
//=============================================================================

std::string RandomString(TRandom &rnd, int len=16)
{
  std::string s;
  s.resize(len);
  for (int i = 0; i < len; ++i)
  {
    int r = rnd.Integer(10 + 26 + 26);
    if (r < 10) {
      s[i] = '0' + r;
    } else {
      r -= 10;
      if (r < 26)
        s[i] = 'A' + r;
      else
        s[i] = 'a' + r - 26;
    }
  }
  return s;
}

//=============================================================================
//=============================================================================

void SendRawString(TSocket *s, const char *msg)
{
   s->SendRaw(msg, strlen(msg));
}

//=============================================================================
//=============================================================================

void revetor()
{
   namespace REX = ROOT::Experimental;

   // Establish signal handlers.
   struct sigaction sa, sa_chld, sa_int, sa_term;
   sigemptyset(&sa.sa_mask);
   sa.sa_flags = 0;
   sa.sa_handler = child_handler;
   sigaction(SIGCHLD, &sa, &sa_chld);

   sa.sa_handler = int_handler;
   sigaction(SIGINT, &sa, &sa_int);
   sigaction(SIGTERM, &sa, &sa_term);

   // Common init. If REveManager supported delayed server startup we could
   // also do REveManager::Create() here.
   TApplication app("fwService", 0, 0);

   TServerSocket *ss = new TServerSocket(FIREWORKS_SERVICE_PORT, kTRUE);
   if ( ! ss->IsValid())
   {
      fprintf(stderr, "Failed creating TServerSocket with code %d\n", ss->GetErrorCode());
      exit(1);
   }
   printf("Server socket created on port %d, listening ...\n", FIREWORKS_SERVICE_PORT);

   // Message queue for child status report
   if ((global_msgq_id = msgget(IPC_PRIVATE, IPC_CREAT | 0660)) == -1)
   {
      perror("msgget for child message queue failed");
      exit(1);
   }
   std::thread msgq_listener_thread( msgq_receiver_thread_foo );

   // ---------------------------------------------------------

   int N_tot_children = 0;

   while (ACCEPT_NEW)
   {
      fd_set read, write, except;
      FD_ZERO(&read);   FD_ZERO(&write);   FD_ZERO(&except);
      FD_SET(ss->GetDescriptor(), &read);
      int max_fd = ss->GetDescriptor();

      int selret = select(max_fd + 1, &read, &write, &except, NULL);

      if (selret == -1)
      {
         const char *fErrorStr;
         switch (errno)
         {
         case 0: // Cancelled ... or sth ...
            fErrorStr = "Unknown error (errno=0).";
            break;
         case EBADF:
            fErrorStr = "Bad file-descriptor.";
            break;
         case EINTR:
            fErrorStr = "Interrupted select.";
            break;
         case EINVAL:
            fErrorStr = "Bad parameters (num fds or timeout).";
            break;
         case ENOMEM:
            fErrorStr = "No memory for select.";
            break;
         default:
            fErrorStr = "Undocumented error in select.";
            break;
         } // end switch
         printf("Select error %d, '%s'\n", errno, fErrorStr);
         continue;
      }

      if (selret == 0) continue;

      if (FD_ISSET(ss->GetDescriptor(), &read))
      {
         TSocket *s = ss->Accept();

         TInetAddress ia = s->GetInetAddress();

         printf("Connection from %s\n", ia.GetHostName());

         SendRawString(s, "Hello, this is Revetor! What do you want?\n");

         char resp[4096];
         int rl = s->RecvRaw(resp, 4096, kDontBlock);
         char *nlp = strpbrk(resp, "\n\r");
         if (nlp) { *nlp = 0; rl = nlp - resp; }
         else
         {
            printf("Bad response, no \\n, terminating connection.\n");
            s->Close();
            delete s;
            continue;
         }
         

         if (rl > 0)
         {
            ++N_tot_children;

            char outerr_fname[64];
            // Probably need something with date and username.
            snprintf(outerr_fname, 64, "revetor-%d.outerr", N_tot_children);

            printf("A guy asking for (resp_len=%d):'%s', blindly doing it.\n"
                   "  Stdout/err will be in 'tail -f %s'\n",
                   rl, resp, outerr_fname);

            pid_t pid = fork();

            if (pid)
            {
               s->Close();
               delete s;

               children[pid] = N_tot_children;
            }
            else
            {
               // We are the child and will reuse the socket to tell back where EVE dude has started.

               sigaction(SIGCHLD, &sa_chld, NULL);
               sigaction(SIGINT,  &sa_int,  NULL);
               sigaction(SIGTERM, &sa_term, NULL);

               // Close the server socket.
               ss->Close();

               // Close stdin, redirect stdout/err
               fclose(stdin);
               stdin = fopen("/dev/null", "r");
               dup2(fileno(stdin), 0);

               fclose(stdout); fclose(stderr);
               stdout = fopen(outerr_fname, "w");
               stderr = stdout;
               dup2(fileno(stdout), 1);
               dup2(fileno(stderr), 2);
               setlinebuf(stdout);

               // Instance init.
               FW2Main fwShow;

               int argc = 2;
               char* argv[2] = { (char*) "fwShow.exe", resp };

               fwShow.parseArguments(argc, argv);

               // QQQQ Can go to common init? Spits krappe
               // gROOT->ProcessLine("#include \"DataFormats/FWLite/interface/Event.h\"");
               
               // What does this do?
               REX::gEve->Show();

               // Loaded, notify remote where to connect.
               auto eve = REX::gEve; // REX::REveManager::Create();

               // Connection key
               TRandom3 rnd(0);
               std::string con_key = RandomString(rnd, 16);
               eve->GetWebWindow()->SetConnToken(con_key);

               auto url = eve->GetWebWindow()->GetUrl();
               std::regex re("(\\w+)://([^:]+):(\\d+)/*(.*)");
               std::smatch m;
               std::regex_search(url, m, re);

               int nm = m.size();
               printf("URL match %d\n", nm);
               for (int i = 0; i < nm; ++i) {
                  printf("  %d: %s\n", i, m[i].str().c_str());
               }

               char pmsg[1024];
               snprintf(pmsg, 1024, "{ 'port'=>%s, 'dir'=>'%s', 'key'=>'%s' }\n",
                        m[3].str().c_str(), m[4].str().c_str(), con_key.c_str());

               SendRawString(s, pmsg);

               s->Close();
               delete s;

               // Start status report timer
               global_server_id = N_tot_children;
               StatReportTimer stat_report_timer;
               stat_report_timer.SetTime(30 * 1000);
               stat_report_timer.Start();

               // Run the standard event loop.
               app.Run();

               // Exit.
               exit(0);
            }
         }
         else
         {
            printf("Hmmh, reponse legth = %d, closing connection\n", rl);
            s->Close();
            delete s;
         }
      }
   }

   // End condition met ... shutdown.

   sigaction(SIGCHLD, &sa_chld, NULL);
   sigaction(SIGINT,  &sa_int,  NULL);
   sigaction(SIGTERM, &sa_term, NULL);

   ss->Close();
   delete ss;

   printf("Exited main loop, still have %d children.\n", (int) children.size());

   for (const auto& [pid, id] : children)
   {
      printf("  Killing child %d, pid=%d\n", id, pid);
      kill(pid, SIGKILL);
   }

   printf("Removing message queue.\n");
   msgctl(global_msgq_id, IPC_RMID, 0);
   msgq_listener_thread.join();

   printf("Revetor exiting\n");
}


int main(int argc, char *argv[])
{
   
   std::string descString(argv[0]);
   descString += " [options] <data file>\nGeneral";
   namespace po = boost::program_options;
   po::options_description desc(descString);
   desc.add_options()(kInputFilesCommandOpt, po::value<std::vector<std::string>>(), "Input root files")(kPortCommandOpt, po::value<unsigned int>(), "Http server port")(kHelpCommandOpt, "Display help message");

   po::positional_options_description p;
   p.add(kInputFilesOpt, -1);

   int newArgc = argc;
   char **newArgv = argv;
   po::variables_map vm;


   try {
      po::store(po::command_line_parser(newArgc, newArgv).
                options(desc).positional(p).run(), vm);

      po::notify(vm);
   }
   catch (const std::exception& e)
   {
      // Return with exit status 0 to avoid generating crash reports

      std::cout <<  e.what() << std::endl;
      std::cout << desc <<std::endl;
      exit(0); 
   }
   
   if(vm.count(kHelpOpt)) {
      std::cout << desc <<std::endl;
      exit(0);
   }

   if (vm.count(kPortCommandOpt)) {
      auto portNum = vm[kPortCommandOpt].as<unsigned int>();
      FIREWORKS_SERVICE_PORT = portNum;
   }

   if (vm.count(kInputFilesOpt)) {
      auto inputFiles = vm[kInputFilesOpt].as<std::vector<std::string> >();
      for(auto fp : inputFiles){
         printf("Pre open file %s\n", fp.c_str());
         auto tf = TFile::Open(fp.c_str());
         tf->Close();
      }
   }

   printf("%s starting: gROOT=%p, http-port=%d, min-port=%d, max-port=%d\n", argv[0], gROOT,
          gEnv->GetValue("WebGui.HttpPort"   , -1),
          gEnv->GetValue("WebGui.HttpPortMin", -1),
          gEnv->GetValue("WebGui.HttpPortMax", -1));

   revetor();

   return 0;
}

/*
int main(int argc, char* argv[])
{

   std::cerr << "THIS IS TEST SERVICE :)\n";

   
   FW2Main fire(argc, argv);
   ROOT::Experimental::gEve->Show();
   
   TApplication app("revetor", 0, 0);


   return 0;
}
*/
