#include <iostream>
#include "TFile.h"
#include "TTree.h"
#include "TEventList.h"
#include "TClass.h"
#include "TROOT.h"
#include "TRint.h"

#include "ROOT/REveDataClasses.hxx"
#include "ROOT/REveManager.hxx"
#include "ROOT/REveScene.hxx"

#include "OssTests/Core/interface/FW2EveManager.h"

#include "OssTests/Core/src/FW2JetProxyBuilder.cc"
#include "OssTests/Core/src/FW2TrackProxyBuilder.cc"
#include "DataFormats/FWLite/interface/Event.h"


namespace REX = ROOT::Experimental;

//========================================================================

struct App
{
   TFile              *m_file;
   TTree              *m_event_tree;
   fwlite::Event      *m_event;
   REX::REveScene     *m_collections;
   FW2EveManager      *m_eveMng;

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

      REX::REveManager::Create();

      m_eveMng = new FW2EveManager();
      m_eveMng->setTableCollection("Tracks");
      
      m_collections =  REX::gEve->SpawnNewScene("Collections","Collections");

      REX::gEve->Show();
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

   std::map<std::string, std::function<void(void* , REX::REveDataCollection*)>> m_item_loader_map;

   void dump_through_loaders()
   {
      // AMT should i loop over m_collections ????
      for (auto & lm_entry : m_item_loader_map)
      {
         printf("dump_through_loaders  %s\n", lm_entry.first.c_str());
         TString cname(lm_entry.first.c_str()); 
         auto col = (REX::REveDataCollection*)m_collections->FindChild(cname);
         lm_entry.second(m_event, col);
      }
      m_eveMng->eventChanged();
   }

   //   void register_std_loader(const std::string &name, const std::string &col_type, const std::string &tag, const std::string &accessor="i")

   void register_std_loader(const std::string &name, const std::string &ctype, const std::string &col_type, const std::string &tag, REX::REveDataProxyBuilderBase* builder, const std::string &accessor="i")
   {
      auto col = new REX::REveDataCollection(name);
      col->SetItemClass(new TClass(ctype.c_str()));
      col->SetMainColor(kGreen);
      m_collections->AddElement(col);

      m_eveMng->registerCollection(col, builder, false);
      
      char buf[2048];
      
      sprintf(buf,
              "*((std::function<void(fwlite::Event*, ROOT::Experimental::REveDataCollection*)>*) %p) = [] (fwlite::Event* ev , ROOT::Experimental::REveDataCollection* col) {"
              "  edm::Handle<%s> handle;   edm::InputTag tag(\"%s\"); "
              " try  {"
              "  ev->getByLabel(tag, handle); "
              "  int cc = 0; "
              "  for (auto & i : *handle) { ++cc;"
              ""
              " std::string iname = Form(\"item %%d\", cc); "
              " col->AddItem( (void*) &i, iname, iname ); "
              "  }"
              "} catch (const cms::Exception& iE) { "
              "   std::cerr << iE.what() <<std::endl;"
              "} };",
              (void*) & m_item_loader_map[name], col_type.c_str(), tag.c_str());
      /*
      sprintf(buf,
              "*((std::function<void(fwlite::Event*, ROOT::Experimental::REveDataCollection*)>*) %p) = [] (fwlite::Event* ev , ROOT::Experimental::REveDataCollection* col) {"
              "  edm::Handle<%s> handle;   edm::InputTag tag(\"%s\"); "
              " try  {"
              "  ev->getByLabel(tag, handle); "
              "  int cc = 0; "
              "  for (auto & i : *handle) { ++cc;"
              ""
              "     printf(\"    %%d: pt = %%.3f\\n\", cc, (%s).pt());"
              " std::string iname = Form(\"item %%d\", cc); "
              " col->AddItem( (void*) &i, iname, iname ); "
              "  }"
              "} catch (const cms::Exception& iE) { "
              "   std::cerr << iE.what() <<std::endl;"
              "} };",
              (void*) & m_item_loader_map[name], col_type.c_str(), tag.c_str(), accessor.c_str());
      */
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

   app.register_std_loader("Tracks", "reco::Track", "reco::TrackCollection",      "generalTracks", new FW2TrackProxyBuilder());
   app.register_std_loader("Jets", "reco::CaloJet",  "std::vector<reco::CaloJet>", "ak4CaloJets", new FW2JetProxyBuilder());

   
   for (Long64_t e = 0; e < 3; ++e)
   {
      printf("========================================\n");
      printf("Event %lld:\n", e);

      app.goto_event(e);

      app.dump_through_loaders();
      break;     
   }


   (new TRint("booname", &argc, argv))->Run();

   return 0;
}
