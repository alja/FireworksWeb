#include <iostream>
#include "TFile.h"
#include "TTree.h"
#include "TEventList.h"
#include "DataFormats/FWLite/interface/Event.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"

#include "DataFormats/JetReco/interface/CaloJet.h"
#include "DataFormats/JetReco/interface/CaloJetCollection.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/BeamSpot/interface/BeamSpot.h"

#include "TClass.h"
//#include "Cintex/Cintex.h"
#include "TROOT.h"
#include "TRint.h"

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

   std::map<std::string, std::function<void(void* /*, REveDataCopllecyion* col */)>> m_item_loader_map;

   void dump_through_loaders()
   {
      for (auto & lm_entry : m_item_loader_map)
      {
         printf("dump_through_loaders  %s\n", lm_entry.first.c_str());

         lm_entry.second(m_event /*, col */ );
      }
   }

   void register_std_loader(const std::string &name, const std::string &col_type, const std::string &tag, const std::string &accessor="i")
   {
      char buf[2048];

      sprintf(buf,
              "*((std::function<void(fwlite::Event*)>*) %p) = [] (fwlite::Event* ev /*, col */) {"
              "  edm::Handle<%s> handle;   edm::InputTag tag(\"%s\"); "
              " try  {"
              "  ev->getByLabel(tag, handle); "
              "  int cc = 0; "
              "  for (auto & i : *handle) { ++cc;"
              "     printf(\"    %%d: pt = %%.3f\\n\", cc, (%s).pt());"
              ""
              " /* col->AddItem( (void*) &i ); */"
              "  }"
              "} catch (const cms::Exception& iE) { "
              "   std::cerr << iE.what() <<std::endl;"
              "} };",
              (void*) & m_item_loader_map[name], col_type.c_str(), tag.c_str(), accessor.c_str());

      printf("BUF\n\n%s\n\n", buf);
      
      gROOT->ProcessLine(buf);
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
   
   App app(argv[1]);

   gROOT->ProcessLine("#include \"DataFormats/FWLite/interface/Event.h\"");

   app.register_std_loader("Tracks", "reco::TrackCollection",      "generalTracks");
   app.register_std_loader("Jets",   "std::vector<reco::CaloJet>", "ak4CaloJets");

   for (Long64_t e = 0; e < 3; ++e)
   {
      printf("========================================\n");
      printf("Event %lld:\n", e);

      app.goto_event(e);

      app.dump_through_loaders();
   }

   (new TRint("booname", &argc, argv))->Run();

   return 0;
}
