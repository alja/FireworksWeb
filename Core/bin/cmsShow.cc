#include <iostream>
#include "TFile.h"
#include "TTree.h"
#include "TEventList.h"
#include "TClass.h"
#include "TROOT.h"
#include "TRint.h"
#include "TApplication.h"


#include "FireworksWeb/Core/interface/FW2Main.h"
#include "ROOT/REveManager.hxx"

//========================================================================

int main(int argc, char* argv[])
{
   // check root interactive promp
   bool isri = false;
   for (Int_t i =0; i<argc; i++)
   {
      if (strncmp(argv[i], "-r", 2) == 0 ||
	  strncmp(argv[i], "--root", 6) == 0)
      {
         isri=true;
      }
   }
    
   TApplication* app;
   const char* dummyArgvArray[] = {argv[0]};
   char** dummyArgv = const_cast<char**>(dummyArgvArray);
   int dummyArgc = 1;
   
   try {
      if (isri) {
         std::cerr<<""<<std::endl;
         std::cerr<<"WARNING:You are running cmsShow with ROOT prompt enabled."<<std::endl;
         std::cerr<<"If you encounter an issue you suspect to be a bug in     "<<std::endl;
         std::cerr<<"cmsShow, please re-run without this option and try to    "<<std::endl;
         std::cerr<<"reproduce it before submitting a bug-report.             "<<std::endl;
         std::cerr<<""<<std::endl;
         app = new TRint("fwShow", &dummyArgc, dummyArgv);
      } else {
         app = new TApplication("fwShow", &dummyArgc, dummyArgv);
      }
   }
   catch(std::exception& iException)
   {
      std::cerr <<"Unhandled exception "<<iException.what()<<std::endl;
      return 1;      
   }


   FW2Main fwMain;
   fwMain.parseArguments(argc, argv);
   ROOT::Experimental::gEve->Show();

   app->Run();

  

   return 0;
}
