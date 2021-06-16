#include "TRandom3.h"
#include "TSystem.h"
#include "TServerSocket.h"
#include "TEnv.h"
#include "TROOT.h"
#include "TApplication.h"

#include "ROOT/REveManager.hxx"
#include "ROOT/RWebWindow.hxx"

#include <cstdio>
#include <string>
#include <regex>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <unistd.h>
#include <signal.h>

#include "FireworksWeb/Core/interface/FW2Main.h"

const int PORT = 6666;

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

   // Establish handler.
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

   TServerSocket *ss = new TServerSocket(PORT, kTRUE);

   printf("Server socket created on port %d, listening ...\n", PORT);

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

               // In principle, we could keep this open and send status info (still active)
               // to server process. Would need to extend fd select and children map and all that.
               s->Close();
               delete s;

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

   // End condition met
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
}


int main(int argc, char *argv[])
{
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
