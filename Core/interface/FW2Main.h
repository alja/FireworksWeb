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

   void goto_event(Long64_t);

   void dump_through_loaders();

   REX::REveDataCollection* register_std_loader(const std::string &name, const std::string &ctype, const std::string &col_type, const std::string &tag, REX::REveDataProxyBuilderBase* builder, const std::string &accessor="i");
   REX::REveDataCollection* register_random_loader();
  

   std::map<std::string, std::function<void(void* , REX::REveDataCollection*)>> m_item_loader_map;

};


class FW2EventManager : public REX::REveElement
{
public:
   Long64_t m_eventId;
   FW2Main* m_app;
   
    std::function<void (Long64_t)> _handler_func;

   FW2EventManager(){ m_eventId = 0;}

   virtual ~FW2EventManager() {}

   void NextEvent() {
      ++m_eventId;
      if ( _handler_func ) 
      {
         _handler_func(m_eventId);
      }
   }
   
   void setHandlerFunc (std::function<void (Long64_t)> handler_func)
   {
      _handler_func = handler_func;
   }
};

#endif
