#define protected public // temporary workaround for missing  ROOT::Experimental::RWebWindow* GetWebWindow() const { return fWebWindow.get(); } in REveManager

#include <TSystem.h>
#include <TServerSocket.h>
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


#include "Fireworks2/Core/interface/FW2Main.h"
#include "ROOT/REveManager.hxx"

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

   const int port = 7777;
   TServerSocket *ss = new TServerSocket(port, kTRUE);

   printf("Server socket created on port %d, listening ...\n", port);

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

               // Run some other event loop, TRint/Application::Run or whatever
               //  gROOT->ProcessLine(".x event_demo.C");
               
               int argc = 2;
               char* argv[2] = {resp, resp};
               FW2Main fire(argc, argv);               
               ROOT::Experimental::gEve->Show();

               
               TApplication app("revetor", 0, 0);


               // Loaded, notify remote where to connect.
               auto eve = REX::gEve; // REX::REveManager::Create();
               //  auto url = eve->GetWebWindow()->GetUrl();
               auto url = eve->fWebWindow->GetUrl();
               std::regex re("(\\w+)://([^:]+):(\\d+)(.*)");
               std::smatch m;
               std::regex_search(url, m, re);

               int nm = m.size();
               printf("URL match %d\n", nm);
               for (int i = 0; i < nm; ++i) {
                  printf("  %d: %s\n", i, m[i].str().c_str());
               }

               char pmsg[256];
               snprintf(pmsg, 256, ":%s%s\n", m[3].str().c_str(), m[4].str().c_str());

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


int main()
{
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
