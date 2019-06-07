#include <iostream>
#include "TFile.h"
#include "TTree.h"
#include "TEventList.h"
#include "TClass.h"
#include "TROOT.h"
#include "TRint.h"


#include "Fireworks2/Core/interface/FW2Main.h"

//========================================================================

int main(int argc, char* argv[])
{
   if (argc < 2)
   {
      std::cout << "Need input file!" << std::endl;
      return 1;
   }
   
   FW2Main app(argv[1]);
   
   gROOT->ProcessLine("#include \"DataFormats/FWLite/interface/Event.h\""); 
   app.printPlugins();


   app.goto_event(1);
      
   ROOT::Experimental::gEve->Show();
   (new TRint("booname", &argc, argv))->Run();

   return 0;
}
