#ifndef FireworksWeb_Core_FW2Main_h
#define FireworksWeb_Core_FW2Main_h

class TTree;
class TFile;

namespace ROOT {
namespace Experimental  {
class REveScene;
}
}

namespace fwlite {
class Event;
}

class FW2EventManager;
class FWEventItem;
class FWItemAccessorFactory;
class FWLiteJobMetadataManager;
class FW2EveManager;
class FW2GUI;
class FWPhysicsObjectDesc;
class FWConfigurationManager;
class FWEventItemsManager;
class FWTableViewManager;

#include "FireworksWeb/Core/interface/CmsShowMainBase.h"


class FW2Main : public CmsShowMainBase
{
public:
   TFile              *m_file;
   TTree              *m_event_tree;
   fwlite::Event      *m_event;
   ROOT::Experimental::REveScene     *m_collections;
   FW2GUI             *m_gui;

   //------------------------------------------------------

   FW2Main();
   ~FW2Main();

   void parseArguments(int argc, char *argv[]);
   void loadInputFiles();
   void nextEvent();
   void previousEvent();
   
   void goto_event(Long64_t);

   const fwlite::Event* getCurrentEvent() const { return m_event; }
   FWLiteJobMetadataManager* getMetadataManager() { return m_metadataManager; }
   FWEventItemsManager* getEventItemsManager() { return m_itemsManager; }
   void addFW2Item(FWPhysicsObjectDesc&);
   
   void setConfigFilename(const std::string &f) { m_configFileName = f; };

private:
   std::string                           m_configFileName;
   
   FWItemAccessorFactory* m_accessorFactory;

   FW2EveManager*              m_eveMng;
   FWLiteJobMetadataManager*   m_metadataManager;
   FWEventItemsManager*        m_itemsManager;
   FWConfigurationManager*     m_configurationManager;
   FWTableViewManager*         m_tableManager;
   Long64_t m_eventId;
   std::vector<std::string> m_inputFiles;

   void setupConfiguration();
};



#endif
