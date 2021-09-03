#include "TRandom3.h"
#include "TSystem.h"
#include "TServerSocket.h"
#include "TEnv.h"
#include "TROOT.h"
#include "TFile.h"
#include "TApplication.h"

#include "ROOT/REveManager.hxx"
#include "ROOT/RWebWindow.hxx"
#include "nlohmann/json.hpp"

#include <cstdio>
#include <ctime>
#include <string>
#include <regex>
#include <mutex>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>

#include <boost/program_options.hpp>
#include "FireworksWeb/Core/interface/FW2Main.h"

static int FIREWORKS_SERVICE_PORT = 6666;
static int FIREWORKS_MAX_SERVERS = 100;
static int FIREWORKS_USER_TIMEOUT = 1800;
static int FIREWORKS_DISCONNECT_TIMEOUT = 600;

static const char* const kPortCommandOpt = "port";
static const char* const kInputFilesOpt = "input-files";
static const char* const kInputFilesCommandOpt = "input-files";
static const char* const kMaxNumServersCommandOpt = "nsrv";
static const char* const kMirIdleTimeCommandOpt = "mir-timeout";
static const char* const kLastDisconnectCommandOpt = "disconnect-timeout";
static const char* const kHelpOpt        = "help";
static const char* const kHelpCommandOpt = "help,h";

//=============================================================================
// Message Queue stuff
//=============================================================================

int         global_msgq_id;
pid_t       global_child_pid = -1; // set for children after fork

// To be renamed to REX::REveServerStatus or something and replace ClientStatus there.
// Has to be POD so we can mem-copy it around and send it via message-queue
struct ChildStatus
{
   pid_t       f_pid = 0;
   int         f_n_connects = 0;
   int         f_n_disconects = 0;
   std::time_t f_t_report = 0;
   std::time_t f_t_last_mir = 0;
   std::time_t f_t_last_connect = 0;
   std::time_t f_t_last_disconnect = 0;
   ProcInfo_t  f_proc_info; // gSystem->GetProcInfo(&cs.f_proc_info);
   // (to be complemented with cpu1/5/15 and memgrowth1/5/15 in collector struct)
   // (or it could be here as well)

   int n_active_connections() const { return f_n_connects - f_n_disconects; }

   // this really goes somewhere else
   float getIdleScore() const
   {
      float score;
      std::time_t now = std::time(nullptr);
      if (n_active_connections() == 0)
      {
         score = 100 * difftime(now, f_t_last_disconnect);
      }
      else
      {
         // if Nconnection == -1, means noone has connected yet, mirTime is the server creation time
         // at the moment both cases are treated the same
         score = difftime(now, f_t_last_mir);
      }
      return score;
   }
};

struct fw_msgbuf {
   long        mtype;  // message type, must be > 0 -- pid of receiving process, 1 for master
   ChildStatus mbody;
};

void msgq_test_send(long id, ROOT::Experimental::REveManager::ClientStatus& rcs)
{
   struct fw_msgbuf msg;
   msg.mtype = id;
   ChildStatus &cs = msg.mbody;
   cs.f_pid = global_child_pid;
   cs.f_n_connects = rcs.fNConnections;
   cs.f_t_last_disconnect = rcs.fDisconnectTime;
   cs.f_t_last_mir = rcs.fMIRTime;
   gSystem->GetProcInfo(&cs.f_proc_info);

   if (msgsnd(global_msgq_id, (void *) &msg, sizeof(msg.mbody), IPC_NOWAIT) == -1) {
      perror("msgsnd error");
      return;
   }
   printf("sent status: N conn = %d\n", msg.mbody.f_n_connects);
}

struct StatReportTimer : public TTimer
{
   bool Notify() override
   {
      using namespace ROOT::Experimental;
      REveManager::ClientStatus cs;
      gEve->GetClientStatus(cs);
      msgq_test_send(1, cs);
      Reset();
      return true;
   }
};

//=============================================================================
// Signal and child process handling
//=============================================================================

struct ChildInfo
{
   ChildStatus  f_last_status;
   pid_t        f_pid;
   int          f_seq_id;
   std::time_t  f_start_time;
   std::time_t  f_end_time; // do we need this
   std::string  f_user;
   std::string  f_log_file;

   ChildInfo() = default;

   ChildInfo(pid_t pid, int sid, const std::string& usr, const std::string& log) :
      f_pid(pid), f_seq_id(sid),
      f_start_time(std::time_t(nullptr)), f_end_time(0),
      f_user(usr), f_log_file(log)
   {}
};

std::mutex g_mutex;
std::map<pid_t, ChildInfo> g_children_map;

static void child_handler(int sig)
{
    pid_t pid;
    int   status;

    printf("Got SigCHLD ... entering waitpid loop.\n");

    while((pid = waitpid(-1, &status, WNOHANG)) > 0)
    { 
       const std::lock_guard<std::mutex> lock(g_mutex);
       auto i = g_children_map.find(pid);
       if (i != g_children_map.end())
       {
          printf("Child pid=%d id=%d died ... cleaning up.\n", i->first, i->second.f_seq_id);
          g_children_map.erase(i);
       }
       else
       {
          printf("Got SigCHLD for pid=%d, not in map, hmmh.\n", pid);
       }
    }
}

void msgq_receiver_thread_foo()
{
   while (true)
   {
      struct fw_msgbuf msg;

      if (msgrcv(global_msgq_id, (void *)&msg, sizeof(msg.mbody), 1, 0) == -1)
      {
         if (errno == EIDRM)
         {
            printf("message queue listener thread terminating on queue removal\n");
            break;
         }
         perror("msgrcv");
      }
      else
      {
         ChildStatus &cs = msg.mbody;
         printf("message received from pid %d, status: (N=%d, t_MIR=%lu, t_Dissconn=%lu)\n",
                cs.f_pid, cs.f_n_connects,
                cs.f_t_last_mir, cs.f_t_last_disconnect);

         const std::lock_guard<std::mutex> lock(g_mutex);
         auto it = g_children_map.find(cs.f_pid);
         if (it != g_children_map.end()) {
            g_children_map[cs.f_pid].f_last_status = cs;
         }
         else {
            printf("Error: child %d can't be located in map\n", cs.f_pid);
         }
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
// Clear idle processes
//=============================================================================
void KillIdleProcesses()
{
   // store results in buffer
   std::vector<ChildStatus> v;
   {
      const std::lock_guard<std::mutex> lock(g_mutex);
      for (const auto &[pid, cinfo] : g_children_map)
      {
         if (cinfo.f_last_status.f_pid)
            v.push_back(cinfo.f_last_status);
         else
            printf("child %d has not initialized status yet", pid);
      }
   }

   // check hard limits without sort
   printf("Checking idle processes ...\n");
   std::time_t now = std::time(nullptr);
   for (auto &s : v) {
      bool doKill = false;
      if (s.f_n_connects == 0) {
         float dt =  difftime(now, s.f_t_last_disconnect);
         doKill = dt > FIREWORKS_DISCONNECT_TIMEOUT;
         printf("pid %d disconnected %f seconds ago\n", s.f_pid, dt);
      }
      else {
         float dt =  difftime(now, s.f_t_last_mir);
         doKill = dt > FIREWORKS_USER_TIMEOUT;
         printf("pid %d user active %f seconds ago\n", s.f_pid, dt);
      }
      if (doKill) {
         printf("Going to kill idle process %d\n", s.f_pid);
         kill(s.f_pid, SIGKILL);
      }
   }
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
         if (rl > 0 && resp[rl - 1] == '\n')
         {
            printf("Got request: %s\n", resp);
            resp[rl - 1] = 0;
            --rl;
         }
         else
         {
            printf("Error, bad response or no \\n (resp_len=%d), terminating connection.\n", rl);
            s->Close();
            delete s;
            continue;
         }

         nlohmann::json req;
         try {
            req = nlohmann::json::parse(resp);
         }
         catch (std::exception &exc) {
            std::cout << "JSON parse caugth exception: " << exc.what() << "\n";
            SendRawString(s, "{'error'=>'json parse'}");
         }

         if (req["action"] == "load")
         {
            KillIdleProcesses();
            ++N_tot_children;

            std::string logdir = req["logdir"].get<std::string>();
            // XXXX stat logdir, create if it does not exist
            {
               struct stat sb;
               if (stat(logdir.c_str(), &sb))
               {
                  if (errno == ENOENT) {
                     printf("logdir %s does not exist, trying to create.\n", logdir.c_str());
                     if (mkdir(logdir.c_str(), 0777)) {
                        printf("  mkdir failed: %s\n", strerror(errno));
                        s->Close(); delete s; continue;
                     }
                  } else {
                     printf("logdir stat failed: %s\n", strerror(errno));
                     s->Close(); delete s; continue;
                  }
               }
               else
               {
                  if ((sb.st_mode & S_IFMT) != S_IFDIR) {
                     printf("logdir is not a directory\n");
                     s->Close(); delete s; continue;
                  }
                  if (access(logdir.c_str(), W_OK))
                  {
                     printf("logdir can not write: %s\n", strerror(errno));
                     s->Close(); delete s; continue;
                  }
               }
            }

            time_t  epoch = time(0);
            struct tm t;
            localtime_r(&epoch, &t);

            pid_t pid = fork();

            if (pid)
            {
               s->Close();
               delete s;

               char log_fname[128];
               snprintf(log_fname, 1024, "%d%02d%02d-%02d%02d%02d-%d.log",
                        1900 + t.tm_year, 1 + t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec,
                        pid);
               logdir += "/";
               logdir += log_fname;

               std::string user = req["user"].get<std::string>();

               g_mutex.lock();
               g_children_map[pid] = ChildInfo(pid, N_tot_children, user, logdir);
               g_mutex.unlock();

               printf("Forked an instance for user %s, log is %s\n", user.c_str(),
                      logdir.c_str());

               continue;
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

               global_child_pid = getpid();

               char log_fname[128];
               snprintf(log_fname, 128, "%d%02d%02d-%02d%02d%02d-%d.log",
                        1900 + t.tm_year, 1 + t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec,
                        global_child_pid);
               logdir += "/"; logdir += log_fname;

               if ((stdout = fopen(logdir.c_str(), "w")) == nullptr) {
                  snprintf(log_fname, 128, "<unable to open: %s>", strerror(errno));
               }
               stderr = stdout;
               dup2(fileno(stdout), 1);
               dup2(fileno(stderr), 2);
               setlinebuf(stdout);

               // Instance init.
               FW2Main fwShow;

               int argc = 2;
               std::string file = req["file"].get<std::string>();
               char* argv[2] = { (char*) "fwShow.exe", (char*) file.c_str() };

               fwShow.parseArguments(argc, argv);

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
               snprintf(pmsg, 1024, "{ 'port'=>%s, 'dir'=>'%s', 'key'=>'%s', 'log_fname'=>'%s' }\n",
                        m[3].str().c_str(), m[4].str().c_str(), con_key.c_str(), log_fname);

               SendRawString(s, pmsg);

               s->Close();
               delete s;

               // Start status report timer
               StatReportTimer stat_report_timer;
               stat_report_timer.SetTime(30 * 1000);
               stat_report_timer.Start();

               // Run the standard event loop.
               app.Run();

               // Exit.
               exit(0);
            }
         }
      }
   }

   // End condition met ... shutdown.

   sigaction(SIGCHLD, &sa_chld, NULL);
   sigaction(SIGINT,  &sa_int,  NULL);
   sigaction(SIGTERM, &sa_term, NULL);

   ss->Close();
   delete ss;

   printf("Exited main loop, still have %d children.\n", (int) g_children_map.size());

   g_mutex.lock();
   for (const auto& [pid, cinfo] : g_children_map)
   {
      printf("  Killing child %d, pid=%d\n", cinfo.f_seq_id, pid);
      kill(pid, SIGKILL);
   }
   g_mutex.unlock();

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
   desc.add_options()(kInputFilesCommandOpt, po::value<std::vector<std::string>>(), "Input root files")
                     (kPortCommandOpt, po::value<unsigned int>(), "Http server port")
                     (kMaxNumServersCommandOpt, po::value<unsigned int>(), "Max number of servers")
                     (kMirIdleTimeCommandOpt, po::value<unsigned int>(), "User idle timeout")
                     (kLastDisconnectCommandOpt, po::value<unsigned int>(), "Last disconnect timout")
                     (kHelpCommandOpt, "Display help message");

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

   if (vm.count(kMaxNumServersCommandOpt)) {
      auto srvNum = vm[kMaxNumServersCommandOpt].as<unsigned int>();
      FIREWORKS_MAX_SERVERS = srvNum;
   }

   if (vm.count(kMirIdleTimeCommandOpt)) {
      auto mirt = vm[kMirIdleTimeCommandOpt].as<unsigned int>();
      FIREWORKS_USER_TIMEOUT = mirt;
   }

   if (vm.count(kLastDisconnectCommandOpt)) {
      auto dt = vm[kLastDisconnectCommandOpt].as<unsigned int>();
      FIREWORKS_DISCONNECT_TIMEOUT = dt;
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
