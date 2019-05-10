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
#include "Fireworks2/Core/interface/FW2Main.h"
#include "Fireworks2/Core/interface/FW2EveManager.h"


namespace REX = ROOT::Experimental;


#include "Fireworks2/Muons/src/FWMuonBuilder.cc" 
#include "Fireworks2/Core/src/FW2JetProxyBuilder.cc"
#include "Fireworks2/Core/src/FW2TrackProxyBuilder.cc" 
#include "Fireworks2/Muons/plugins/FWMuonProxyBuilder.cc" 


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

   {
      auto col = app.register_std_loader("Tracks", "reco::Track", "reco::TrackCollection",      "generalTracks", new FW2TrackProxyBuilder());
      col->SetMainColor(kGreen);
   }

   {
      auto col = app.register_std_loader("Jets", "reco::CaloJet",  "std::vector<reco::CaloJet>", "ak4CaloJets", new FW2JetProxyBuilder());
      col->SetMainColor(kYellow);
   }
   
   {
      auto col = app.register_std_loader("Muons", "reco::Muon",  "std::vector<reco::Muon>", "muons", new FWMuonProxyBuilder());
      col->SetMainColor(kRed);
   }

   
   app.goto_event(0);
      
   REX::gEve->Show();
   (new TRint("booname", &argc, argv))->Run();

   return 0;
}
