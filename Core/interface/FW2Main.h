#ifndef Fireworks2_Core_FW2Main_h
#define Fireworks2_Core_FW2Main_h


#include "TTree.h"
#include "TFile.h"

#include "ROOT/REveElement.hxx"
#include "ROOT/REveManager.hxx"
#include "ROOT/REveScene.hxx"

#include "Fireworks2/Core/interface/FW2EveManager.h"

#include "DataFormats/FWLite/interface/Event.h"

namespace REX = ROOT::Experimental;

class FW2EventManager;
class FWEventItem;
class FWItemAccessorFactory;
class FWLiteJobMetadataManager;

//========================================================================
class FW2Main
{
public:
   TFile              *m_file;
   TTree              *m_event_tree;
   fwlite::Event      *m_event;
   REX::REveScene     *m_collections;
   FW2EveManager      *m_eveMng;
   FW2EventManager    *m_eventMng;

   //------------------------------------------------------

   FW2Main(const char* fname);
   ~FW2Main();

   void nextEvent();
   void goto_event(Long64_t);


   void addTestItems();

private:
   Long64_t m_eventId;
   FWItemAccessorFactory* m_accessorFactory;
   std::vector <FWEventItem*> m_items;

   std::unique_ptr<FWLiteJobMetadataManager>   m_metadataManager;
};

//==============================================================================
//============= temprary test class till custom GUI ============================
//==============================================================================


class FW2EventManager : public REX::REveElement
{
public:
   std::function<void (void)> _handler_func;
   FW2EventManager(){}
   virtual ~FW2EventManager() {}
   
   void NextEvent() {
      _handler_func();
   }   

   void setHandlerFunc (std::function<void ()> handler_func)
   {
      _handler_func = handler_func;
   }
};

#endif
