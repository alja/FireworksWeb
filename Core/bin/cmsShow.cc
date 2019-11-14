#include <iostream>
#include "TFile.h"
#include "TTree.h"
#include "TEventList.h"
#include "TClass.h"
#include "TROOT.h"
#include "TRint.h"


#include "Fireworks2/Core/interface/FW2Main.h"
#include "ROOT/REveManager.hxx"

//========================================================================

int main(int argc, char* argv[])
{
   if (argc < 2)
   {
      std::cout << "Need input file!" << std::endl;
      return 1;
   }

   
   FW2Main app(argc, argv);
   
   gROOT->ProcessLine("#include \"DataFormats/FWLite/interface/Event.h\""); 

   const char* dummyArgvArray[] = {argv[0]};
   char** dummyArgv = const_cast<char**>(dummyArgvArray);
   int dummyArgc = 1;
      
   ROOT::Experimental::gEve->Show();
   (new TRint("fire", &dummyArgc, dummyArgv))->Run();

   return 0;
}
