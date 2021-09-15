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

namespace REX = ROOT::Experimental;

//=============================================================================
// Message Queue stuff
//=============================================================================

int         N_tot_children = 0;

int         global_msgq_id;
pid_t       global_child_pid = -1; // set for children after fork


struct fw_msgbuf {
   long                  mtype;  // message type, must be > 0 -- pid of receiving process, 1 for master
   REX::REveServerStatus mbody;
};

void msgq_test_send(long id, REX::REveServerStatus& rss)
{
   struct fw_msgbuf msg;
   msg.mtype = id;
   msg.mbody = rss;

   if (msgsnd(global_msgq_id, (void *) &msg, sizeof(msg.mbody), IPC_NOWAIT) == -1) {
      perror("msgsnd error");
      return;
   }
   // printf("sent status: N conn = %d\n", msg.mbody.f_n_connects);
}

struct StatReportTimer : public TTimer
{
   bool Notify() override
   {
      REX::REveServerStatus ss;
      REX::gEve->GetServerStatus(ss);
      msgq_test_send(1, ss);
      Reset();
      return true;
   }
};

//=============================================================================
// Signal and child process handling
//=============================================================================

struct ChildInfo
{
   REX::REveServerStatus  fLastStatus;
   pid_t                  fPid;
   int                    fSeqId;
   std::time_t            fStartTime;
   std::time_t            fEndTime; // do we need this? if we want to keep a list of recent sessions, we do
   std::string            fUser;
   std::string            fLogFile;

   ChildInfo() = default;

   ChildInfo(pid_t pid, int sid, const std::string& usr, const std::string& log) :
      fPid(pid), fSeqId(sid),
      fStartTime(std::time(nullptr)), fEndTime(0),
      fUser(usr), fLogFile(log)
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
          printf("Child pid=%d id=%d died ... cleaning up.\n", i->first, i->second.fSeqId);
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
         REX::REveServerStatus &ss = msg.mbody;
         if (false) {
            std::time_t now = std::time(nullptr);
            auto ttt = [=](std::time_t t) -> double { if (t==0) return -999; return std::difftime(now, t)/60; };
            printf("message from pid %d: dt_start=%.1f, N_conn=%d, N_disconn=%d,"
                   " dt_last_mir=%.1f, dt_last_conn=%.1f, dt_last_dissconn=%.1f minutes ago\n",
                   ss.fPid, ttt(ss.fTStart), ss.fNConnects, ss.fNDisconnects,
                   ttt(ss.fTLastMir), ttt(ss.fTLastConnect), ttt(ss.fTLastDisconnect));
         }
         const std::lock_guard<std::mutex> lock(g_mutex);
         auto it = g_children_map.find(ss.fPid);
         if (it != g_children_map.end()) {
            g_children_map[ss.fPid].fLastStatus = ss;
         } else {
            printf("Error: child %d can't be located in map\n", ss.fPid);
         }
      }
   }
}

//=============================================================================
// Fill out html table report
//=============================================================================

void html_report(std::ostringstream &oss)
{
   std::vector<ChildInfo> v;
   {
      const std::lock_guard<std::mutex> lock(g_mutex);
      v.reserve(g_children_map.size());

      oss << "{ 'total_sessions'=>" << N_tot_children << ", 'current_sessions'=>" << g_children_map.size() <<  ",\n";
      oss << " 'table'=>'<style> table, th, td { border: 1px solid black; padding: 5px; } </style> <table>\n";
      oss << "<tr><th>pid</th><th>dt_start[min]</th><th>N_conn</th><th>N_disconn</th><th>dt_last_mir[min]</th>"
          <<     "<th>dt_last_conn[min]</th><th>dt_last_dissconn [min]</th>"
          <<     "<th>user</th><th>log file</th></tr>\n";

      for (const auto &[pid, cinfo] : g_children_map)
      {
         if (cinfo.fLastStatus.fPid)
            v.push_back(cinfo);
      }
   }
   std::sort(v.begin(), v.end(), [](auto &a, auto &b){ return a.fStartTime < b.fStartTime; });
   std::time_t now = std::time(nullptr);
   auto ttt = [=](std::time_t t) -> double { if (t==0) return -999; return std::difftime(now, t)/60; };
   char tbl_line[1024];
   for (auto &ci : v)
   {
      auto &ss = ci.fLastStatus;
      snprintf(tbl_line, 1024,
               "<tr><td>%d</td><td>%.1f</td><td>%d</td><td>%d</td>"
               "<td>%.1f</td><td>%.1f</td><td>%.1f</td>"
               "<td>%s</td><td><a href=\"%s\">%s</a></td></tr>\n",
               ss.fPid, ttt(ss.fTStart), ss.fNConnects, ss.fNDisconnects,
               ttt(ss.fTLastMir), ttt(ss.fTLastConnect), ttt(ss.fTLastDisconnect),
               ci.fUser.c_str(), ci.fLogFile.c_str(), ci.fLogFile.c_str());
      oss << tbl_line;
   }
   oss << "</table>' }\n";
}

//=============================================================================
// Clear idle processes
//=============================================================================
// Returns number of supposedly active processes (assuming sig-kill will work).

int KillIdleProcesses()
{
   // store results in buffer
   std::vector<REX::REveServerStatus> v;
   int num_active;
   {
      const std::lock_guard<std::mutex> lock(g_mutex);
      num_active = (int) g_children_map.size();
      for (const auto &[pid, cinfo] : g_children_map)
      {
         if (cinfo.fLastStatus.fPid)
            v.push_back(cinfo.fLastStatus);
         else
            printf("Info: child %d has not initialized status yet.\n", pid);
      }
   }
   std::sort(v.begin(), v.end(), [](auto &a, auto &b){ return a.fTStart < b.fTStart; });

   // check hard limits without sort
   printf("Num servers [%d]. Checking idle processes ...\n", num_active);
   std::time_t now = std::time(nullptr);
   for (auto &s : v)
   {
      // QQQQ - to be revisited now with additional data.

      int t_fac = num_active > FIREWORKS_MAX_SERVERS * 9 / 10 ? 1 : 2;

      bool doKill = false;
      if (s.n_active_connections() == 0) {
         float dt = std::difftime(now, s.fNConnects > 0 ? s.fTLastDisconnect : s.fTStart);
         if ((doKill = dt > t_fac * FIREWORKS_DISCONNECT_TIMEOUT))
            printf("Going to kill pid %d disconnected %f seconds ago\n", s.fPid, dt);
      }
      else {
         double dt = std::difftime(now, s.fTLastMir);
         if ((doKill = dt > t_fac * FIREWORKS_USER_TIMEOUT))
            printf("Going to kill pid %d, N connections %d, last client active %.0f seconds ago\n", s.fPid, s.fNConnects, dt);
      }
      if (doKill) {
         --num_active;
         kill(s.fPid, SIGKILL);
      }
   }
   return num_active;
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

bool ACCEPT_NEW = true;

static void int_handler(int sig)
{
    printf("Got SigINT/TERM, exiting main loop, will reap children there.\n");
    ACCEPT_NEW = false;
}

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
            resp[rl - 1] = 0;
            printf("Got request: %s\n", resp);
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

         if (req["action"] == "status")
         {
            char pmsg[1024];
            const std::lock_guard<std::mutex> lock(g_mutex);
            snprintf(pmsg, 1024, "{ 'total_sessions'=>%d, 'current_sessions'=>%d }\n",
                     N_tot_children, (int) g_children_map.size());
            SendRawString(s, pmsg);
            s->Close();
            delete s;
         }
         else if (req["action"] == "report_usage")
         {
            std::ostringstream oss;
            html_report(oss);
            SendRawString(s, oss.str().data());
            s->Close();
            delete s;
         }
         else if (req["action"] == "load")
         {
            int n_active = KillIdleProcesses();

            if (n_active >= FIREWORKS_MAX_SERVERS)
            {
               char pmsg[1024];
               snprintf(pmsg, 1024, "{ 'error'=>'Maximum number of servers reached (%d).' }\n",
                        FIREWORKS_MAX_SERVERS);
               SendRawString(s, pmsg);
               s->Close(); delete s; continue;
            }

            ++N_tot_children;

            std::string logdir = req["logdir"].get<std::string>();
            {
               bool log_fail = false;
               struct stat sb;
               if (stat(logdir.c_str(), &sb))
               {
                  if (errno == ENOENT) {
                     printf("logdir %s does not exist, trying to create.\n", logdir.c_str());
                     if (mkdir(logdir.c_str(), 0777)) {
                        printf("  mkdir failed: %s\n", strerror(errno));
                        log_fail = true;
                     }
                  } else {
                     printf("logdir stat failed: %s\n", strerror(errno));
                     log_fail = true;
                  }
               }
               else
               {
                  if ((sb.st_mode & S_IFMT) != S_IFDIR) {
                     printf("logdir is not a directory\n");
                     log_fail = true;
                  }
                  if (access(logdir.c_str(), W_OK))
                  {
                     printf("logdir can not write: %s\n", strerror(errno));
                     log_fail = true;
                  }
               }
               if (log_fail)
               {
                  char pmsg[1024];
                  snprintf(pmsg, 1024, "{ 'error'=>'Failure creating log file/directory. This is a service misconfiguration.' }\n");
                  SendRawString(s, pmsg);
                  s->Close(); delete s; continue;
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
               char* argv[2] = { (char*) "cmsShowWeb.exe", (char*) file.c_str() };

               try {
                  fwShow.parseArguments(argc, argv);
               }
               catch (std::exception &exc) {
                  char pmsg[1024];
                  snprintf(pmsg, 1024, "{ 'error'=>'%s', 'log_fname'=>'%s' }\n",
                           exc.what(), log_fname);
                  SendRawString(s, pmsg);
                  s->Close();
                  delete s;
                  printf("Exiting after configuration processing error.\n");
                  exit(1);
               }

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

               {
                  char pmsg[1024];
                  snprintf(pmsg, 1024, "{ 'port'=>%s, 'dir'=>'%s', 'key'=>'%s', 'log_fname'=>'%s' }\n",
                           m[3].str().c_str(), m[4].str().c_str(), con_key.c_str(), log_fname);
                  SendRawString(s, pmsg);
               }
               s->Close();
               delete s;

               // Start status report timer
               StatReportTimer stat_report_timer;
               stat_report_timer.SetTime(30 * 1000);
               stat_report_timer.Start();

               // Run the standard event loop.
               app.Run();

               // Exit.
               printf("Exiting cmsShowWeb.\n");
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
      printf("  Killing child %d, pid=%d\n", cinfo.fSeqId, pid);
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
