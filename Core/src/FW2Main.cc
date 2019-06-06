#include  "Fireworks2/Core/interface/FW2Main.h"
#include  "Fireworks2/Core/interface/Context.h"
#include  "Fireworks2/Core/interface/FWGeometry.h"
#include  "Fireworks2/Core/interface/FWMagField.h"
#include "Fireworks2/Core/interface/FWProxyBuilderFactory.h"

#include "DataFormats/FWLite/interface/Event.h"
#include "TROOT.h"

#include <boost/bind.hpp>
#include "ROOT/REveDataProxyBuilderBase.hxx"

// system include files
#include "FWCore/PluginManager/interface/PluginFactory.h"
#include "FWCore/Utilities/interface/ObjectWithDict.h"
#include "DataFormats/CSCRecHit/interface/CSCSegmentCollection.h"

#include "FWCore/Utilities/interface/Exception.h"

#include "FWCore/PluginManager/interface/PluginManager.h"
#include "FWCore/PluginManager/interface/standard.h"
#include "FWCore/Utilities/interface/Exception.h"

using namespace ROOT::Experimental;
FW2Main::FW2Main(const char* fname):
   m_eventMng(0)
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

   edmplugin::PluginManager::configure(edmplugin::standard::config());
   REX::REveManager::Create();

   auto geom = new FWGeometry();
   geom->loadMap("cmsGeom10.root");

   auto context = new fireworks::Context();
   context->initEveElements();
   context->setGeom(geom);
   context->getField()->checkFieldInfo(m_event);

   m_collections =  REX::gEve->SpawnNewScene("Collections","Collections");
   
   m_eveMng = new FW2EveManager();
   m_eveMng->setTableCollection("Tracks"); // temorary here, should be in collection      
      
   m_eventMng = new FW2EventManager();
   m_eventMng->SetName("EventManager");
   REX::gEve->GetWorld()->AddElement(m_eventMng);
   REX::gEve->GetWorld()->AddCommand("NextEvent", "sap-icon://step", m_eventMng, "NextEvent()");
   m_eventMng->setHandlerFunc([=] (Long64_t id) { this->goto_event(id);});
}

FW2Main::~FW2Main()
{
   delete m_event;
   delete m_file;
}

void FW2Main::printPlugins()
{
   std::vector<edmplugin::PluginInfo> available = FWProxyBuilderFactory::get()->available();  
   for (auto &i : available) {
      std::cout << " available plugin ========= " <<  i.name_ << std::endl;
   }

   std::cout << "category " << FWProxyBuilderFactory::get()->category() << std::endl;
   try {
      if(edmplugin::PluginManager::get()->categoryToInfos().end()!=edmplugin::PluginManager::get()->categoryToInfos().find(FWProxyBuilderFactory::get()->category()))
      {
         std::vector<edmplugin::PluginInfo> ac = edmplugin::PluginManager::get()->categoryToInfos().find(FWProxyBuilderFactory::get()->category())->second;
         for (auto &i : ac) {
            std::cout << " from manager plugin ========= " <<  i.name_ << std::endl;
         }
      }
   }
   catch (const cms::Exception& iE){
      std::cout << iE << std::endl;
   }
   
}

void FW2Main::dump_through_loaders()
{
   m_eveMng->beginEvent();
   // AMT should i loop over m_collections ????
   for (auto & lm_entry : m_item_loader_map)
   {
      printf("-----------dump_through_loaders  %s\n", lm_entry.first.c_str());
      TString cname(lm_entry.first.c_str()); 
      auto col = (REX::REveDataCollection*)m_collections->FindChild(cname);
      col->ClearItems();
      col->DestroyElements();

      lm_entry.second(m_event, col);
      col->ApplyFilter();
   }
   m_eveMng->endEvent();
}

void FW2Main::goto_event(Long64_t tid)
{
   m_eventMng->m_eventId = tid;
   m_event->to(tid);
   m_event_tree->LoadTree(tid);
   dump_through_loaders();
}

REX::REveDataCollection* FW2Main::register_std_loader(const std::string &name, const std::string &ctype, const std::string &col_type, const std::string &tag, REX::REveDataProxyBuilderBase* builder, const std::string &accessor)
{
   auto col = new REX::REveDataCollection(name);
      col->SetItemClass(new TClass(ctype.c_str()));
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
              "     printf(\"    %%d: \\n\", cc);"
              " std::string iname = Form(\"item %%d\", cc); "
              " col->AddItem( (void*) &i, iname, iname ); "
              "  }"
              "} catch (const cms::Exception& iE) { "
              "   std::cerr << iE.what() <<std::endl;"
              "} };",
              (void*) & m_item_loader_map[name], col_type.c_str(), tag.c_str());
      */

      
      printf("BUF\n\n%s\n\n", buf);
      
      gROOT->ProcessLine(buf);

      return col;
}


REX::REveDataCollection* FW2Main::register_random_loader()
{
  
   auto col = new REX::REveDataCollection("CSC-segement");
   col->SetItemClass(new TClass("CSCSegment"));
   m_collections->AddElement(col);
   /*
   try {
      edm::Handle<CSCSegmentCollection> handle;
      edm::InputTag tag("cscSegments");
      m_event->getByLabel(tag, handle);

      std::cout << "CSC size " << handle->size() << "==========+++++\n";
      for (auto & i : *handle) {
         std::cout << "andle CSC =================================\n";
         i.print();
      }
      
   }
   catch (const cms::Exception& iE) {
       std::cerr << iE.what() <<std::endl;
   }
*/
   return col;
}



//==============================================================================

//==============================================================================

//==============================================================================
