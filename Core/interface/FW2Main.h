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
class FWAssociationManager;
class FWTableViewManager;
class CmsShowNavigator;

#include <thread>

#include "FireworksWeb/Core/interface/CmsShowMainBase.h"
#include "FireworksWeb/Core/interface/FWGeometry.h"


class FW2Main : public CmsShowMainBase
{
public:
   //------------------------------------------------------

   FW2Main(bool standalone = true);
   ~FW2Main();

   void parseArguments(int argc, char *argv[]);
   void setupDataHandling();
   void nextEvent();
   void previousEvent();
   void firstEvent();
   void lastEvent();
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

   const FWConfigurationManager* getConfigurationManager() {return m_configurationManager;}

   bool isStandalone() const { return m_standalone; }
private:
   ROOT::Experimental::REveScene *m_collections{nullptr};
   FW2GUI *m_gui{nullptr};
   std::unique_ptr<CmsShowNavigator> m_navigator;
   std::unique_ptr<fireworks::Context> m_context;
   std::string m_configFileName;

   FWItemAccessorFactory *m_accessorFactory{nullptr};

   FW2EveManager *m_eveMng{nullptr};
   FWLiteJobMetadataManager *m_metadataManager{nullptr};
   FWEventItemsManager *m_itemsManager{nullptr};
   FWAssociationManager *m_associationManager{nullptr};
   FWConfigurationManager *m_configurationManager{nullptr};
   FWTableViewManager *m_tableManager{nullptr};
   Long64_t m_eventId;

   FWGeometry m_geom;
   std::string m_geometryFilename;

   bool m_standalone {true};

   std::vector<std::string> m_inputFiles;
   bool m_noVersionCheck{true};
   const TFile *m_openFile{nullptr};

   void draw_event();

   void setupConfiguration();
   void fileChangedSlot(const TFile *file);
   void eventChangedSlot();

};

#endif
