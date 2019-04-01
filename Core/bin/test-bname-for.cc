#include <iostream>
#include "TFile.h"
#include "TTree.h"
#include "TEventList.h"
#include "TClass.h"
#include "TROOT.h"
#include "TRint.h"
/*
#include "ROOT/REveDataClasses.hxx"
#include "ROOT/REveManager.hxx"
#include "ROOT/REveScene.hxx"
*/
#include "OssTests/Core/interface/FW2Main.h"
#include "OssTests/Core/interface/FW2EveManager.h"


namespace REX = ROOT::Experimental;




//========================================================================

int main(int argc, char* argv[])
{
   if (argc < 2)
   {
      std::cout << "Need input file!" << std::endl;
      return 1;
   }
   
   FW2Main app(argv[1]);

   app.goto_event(0);
      
   REX::gEve->Show();
   (new TRint("booname", &argc, argv))->Run();

   return 0;
}
