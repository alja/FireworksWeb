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

#include "Fireworks2/Tracks/plugins/FWTrackProxyBuilder.cc" 
#include "Fireworks2/Muons/plugins/FWMuonProxyBuilder.cc" 
#include "Fireworks2/Muons/plugins/FWCSCSegmentProxyBuilder.cc" 
#include "Fireworks2/Calo/plugins/FWJetProxyBuilder.cc"
#include "Fireworks2/Calo/plugins/FWMETProxyBuilder.cc"
#include "Fireworks2/Electrons/plugins/FWElectronProxyBuilder.cc"



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
      auto col = app.register_std_loader("Jets", "reco::CaloJet",  "std::vector<reco::CaloJet>", "ak4CaloJets", new FWJetProxyBuilder());
      col->SetMainColor(kBlue);
   }
   
   {
      auto col = app.register_std_loader("Muons", "reco::Muon",  "std::vector<reco::Muon>", "muons", new FWMuonProxyBuilder());
      col->SetMainColor(kRed);
   }
   {
      auto col = app.register_std_loader("MET", "reco::PFMET",  "std::vector<reco::PFMET>", "pfMet", new FWMETProxyBuilder());
      col->SetMainColor(kRed);
   }
   
   {
      auto col = app.register_std_loader("CSC-segments", "CSCSegment",  "CSCSegmentCollection", "cscSegments", new FWCSCSegmentProxyBuilder());
      col->SetMainColor(kBlue);
   }
{
      auto col = app.register_std_loader("Tracks", "reco::Track", "reco::TrackCollection",      "generalTracks", new FWTrackProxyBuilder());
      col->SetFilterExpr("i.pt() > 1");
      col->SetMainColor(kGreen + 2);
   }

   /*
   {
      auto col = app.register_std_loader("Electrons", "reco::GsfElectron",  "std::vector<reco::GsfElectron>", "gedGsfElectrons", new FWElectronProxyBuilder());
      col->SetMainColor(kCyan);
      }
   */


   app.goto_event(1);
      
   REX::gEve->Show();
   (new TRint("booname", &argc, argv))->Run();

   return 0;
}
