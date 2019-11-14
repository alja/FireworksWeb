#include <iostream>
#include "TFile.h"
#include "TTree.h"
#include "TEventList.h"
#include "TClass.h"
#include "TROOT.h"
#include "TRint.h"
#include "TApplication.h"


#include "Fireworks2/Core/interface/FW2Main.h"
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
   
   FW2Main app(argc, argv);
   
   gROOT->ProcessLine("#include \"DataFormats/FWLite/interface/Event.h\""); 

   const char* dummyArgvArray[] = {argv[0]};
   char** dummyArgv = const_cast<char**>(dummyArgvArray);
   int dummyArgc = 1;

   ROOT::Experimental::gEve->Show();
   //   (new TRint("fire", &dummyArgc, dummyArgv))->Run();


   
   try {
      if (isri) {
         std::cerr<<""<<std::endl;
         std::cerr<<"WARNING:You are running cmsShow with ROOT prompt enabled."<<std::endl;
         std::cerr<<"If you encounter an issue you suspect to be a bug in     "<<std::endl;
         std::cerr<<"cmsShow, please re-run without this option and try to    "<<std::endl;
         std::cerr<<"reproduce it before submitting a bug-report.             "<<std::endl;
         std::cerr<<""<<std::endl;
         (new TRint("fire", &dummyArgc, dummyArgv))->Run();
      } else {
         (new TApplication("fire", &dummyArgc, dummyArgv))->Run();
      }
   }
   catch(std::exception& iException)
   {
      std::cerr <<"CmsShowWeb unhandled exception "<<iException.what()<<std::endl;
      return 1;      
   }

   return 0;
}
