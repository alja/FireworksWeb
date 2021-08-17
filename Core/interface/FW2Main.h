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
namespace fireworks {
class Context;
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
class CmsShowNavigator;

#include <thread>

#include "FireworksWeb/Core/interface/CmsShowMainBase.h"
#include "FireworksWeb/Core/interface/FWGeometry.h"


class FW2Main : public CmsShowMainBase
{
public:
   //------------------------------------------------------

   FW2Main();
   ~FW2Main();

   void parseArguments(int argc, char *argv[]);
   void setupDataHandling();
   void nextEvent();
   void previousEvent();
   void goToRunEvent(int run, int lumi, int event);
   void postFiltering(bool doDraw);


   const fwlite::Event* getCurrentEvent() const;
   const char* getFrameTitle() const;

   FWLiteJobMetadataManager *getMetadataManager() { return m_metadataManager; }
   FWEventItemsManager *getEventItemsManager() { return m_itemsManager; }
   void addFW2Item(FWPhysicsObjectDesc &);

   void setConfigFilename(const std::string &f) { m_configFileName = f; };
   void doExit();
   void quit();

   bool getVersionCheck() const { return !m_noVersionCheck; }
private:
   ROOT::Experimental::REveScene *m_collections{nullptr};
   FW2GUI *m_gui{nullptr};
   std::unique_ptr<CmsShowNavigator> m_navigator;
   std::unique_ptr<fireworks::Context> m_context;
   std::string m_configFileName;

   FWItemAccessorFactory *m_accessorFactory;

   FW2EveManager *m_eveMng;
   FWLiteJobMetadataManager *m_metadataManager;
   FWEventItemsManager *m_itemsManager;
   FWConfigurationManager *m_configurationManager;
   FWTableViewManager *m_tableManager;
   Long64_t m_eventId;

   FWGeometry m_geom;
   std::string m_geometryFilename;

   std::vector<std::string> m_inputFiles;
   bool m_loadedAnyInputFile{false};
   bool m_noVersionCheck{false};
   const TFile *m_openFile{nullptr};

   void draw_event();

   void setupConfiguration();
   void fileChangedSlot(const TFile *file);
   void eventChangedSlot();

};

#endif
