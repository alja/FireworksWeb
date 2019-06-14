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

//test
#include "FWCore/Utilities/interface/TypeWithDict.h"
#include "Fireworks2/Core/interface/FWSimpleRepresentationChecker.h"
#include "Fireworks2/Core/interface/FWDisplayProperties.h"
#include "Fireworks2/Core/interface/FWPhysicsObjectDesc.h"
#include "Fireworks2/Core/interface/FWEventItem.h"
#include "Fireworks2/Core/interface/FWItemAccessorBase.h"
#include "Fireworks2/Core/interface/FWItemAccessorFactory.h"
#include "Fireworks2/Core/interface/FWPhysicsObjectDesc.h"

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

void FW2Main::goto_event(Long64_t tid)
{
   m_eventMng->m_eventId = tid;
   m_event->to(tid);
   m_event_tree->LoadTree(tid);

   m_eveMng->beginEvent();
   for (auto & item : m_items) {
      item->setEvent(m_event);
   }
   
   m_eveMng->endEvent();
}


void FW2Main::addTestItems()
{
   FWPhysicsObjectDesc desc("TracksName",  TClass::GetClass("std::vector<reco::Track>"), "purpose...", FWDisplayProperties::defaultProperties, "generalTracks" );
   FWEventItem* item = new FWEventItem(m_accessorFactory->accessorFor(desc.type()), desc);
   m_items.push_back(item);
   m_eveMng->newItem(item); 
                                       

}
