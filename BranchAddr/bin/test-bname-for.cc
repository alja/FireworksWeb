#include <iostream>
#include "TFile.h"
#include "TTree.h"
#include "TEventList.h"
#include "DataFormats/FWLite/interface/Event.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/BeamSpot/interface/BeamSpot.h"

#include "TClass.h"
#include "Cintex/Cintex.h"

//========================================================================

struct App
{
   TFile              *m_file;
   TTree              *m_event_tree;
   fwlite::Event      *m_event;

   //------------------------------------------------------

   App(const char* fname)
   {
      m_file = TFile::Open(fname);
      m_event_tree = dynamic_cast<TTree*>(m_file->Get("Events"));
      m_event = 0;
      try
      {
         m_event = new fwlite::Event(m_file);
      }
      catch (const cms::Exception& iE)
      {
         printf("can't create a fwlite::Event\n");
         std::cerr << iE.what() <<std::endl;
         throw;
      }
   }

   ~App()
   {
      delete m_event;
      delete m_file;
   }

   //------------------------------------------------------

   void goto_event(Long64_t tid)
   {
      m_event->to(tid);
      m_event_tree->LoadTree(tid);
   }

   //------------------------------------------------------

   const reco::BeamSpot* checkBeamSpot()
   { 
      try
      {
         edm::InputTag tag("offlineBeamSpot");
         edm::Handle<reco::BeamSpot> spot;

         m_event->getByLabel(tag, spot);
         if (spot.isValid())
         {
            return spot.product();
         }
      }
      catch (cms::Exception& iException)
      {
         std::cerr <<"Can't get beam spot info. Setting coordintes to (0, 0, 0).\n";
      }

      return 0;
   }

   //------------------------------------------------------

   void dump_some_event_stuff()
   {
      const reco::BeamSpot *bs = checkBeamSpot();
      printf("  BeamSpot: %f %f %f (addr=%p)\n", bs->x0(), bs->y0(), bs->z0(), bs);

      // print first few tracks
      edm::Handle<reco::TrackCollection> handle_tracks;
      edm::InputTag tag("generalTracks");
      try
      {
         m_event->getByLabel(tag, handle_tracks);

         const reco::TrackCollection *tracks = &*handle_tracks;
         printf("  First two entries from general tracks (addr=%p) \n", (void*)tracks);

         for (int i = 0; i < 2; ++i)
         {
            printf("    %d: pt = %.3f\n", i, tracks->at(i).pt());
         }
      }
      catch (const cms::Exception& iE)
      {
         std::cerr << iE.what() <<std::endl;
      }
   }

};

//========================================================================

int main(int argc, char* argv[])
{
   if (argc < 2)
   {
      std::cout << "Need input file!" << std::endl;
      return 1;
   }

   ROOT::Cintex::Cintex::Enable();

   App app(argv[1]);

 
   for (Long64_t e = 0; e < 3; ++e)
   {
      printf("========================================\n");
      printf("Event %lld:\n", e);

      app.goto_event(e);

      app.dump_some_event_stuff();
   }

   // ------------------------------------------------------------------

   // m_event->getBranchNameFor(*(item->type()->GetTypeInfo()),
   //                           item->moduleLabel().c_str(),
   //                           item->productInstanceLabel().c_str(),
   //                           item->processName().c_str()); 
   {
      printf("\n");

      std::string bsbname = app.m_event->getBranchNameFor(typeid(reco::BeamSpot), "offlineBeamSpot", "", "RECO");

      printf("BeamSpot branchname = %s\n", bsbname.c_str());

      std::string tkbname = app.m_event->getBranchNameFor(typeid(reco::TrackCollection), "generalTracks", "", "RECO");

      printf("\nTrackCollection branchname = %s\n", tkbname.c_str());

      printf("\n");
   }

   // ------------------------------------------------------------------

   for (Long64_t e = 0; e < 3; ++e)
   {
      printf("========================================\n");
      printf("Event %lld:\n", e);

      app.goto_event(e);

      app.dump_some_event_stuff();
   }

   return 0;
}
