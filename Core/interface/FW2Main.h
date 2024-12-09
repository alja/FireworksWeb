#ifndef FireworksWeb_Core_FW2Main_h
#define FireworksWeb_Core_FW2Main_h

class TTree;
class TFile;
class TMonitor;
class MyMon;

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
class FWWebEventItem;
class FWItemAccessorFactory;
class FWLiteJobMetadataManager;
class FW2EveManager;
class FW2GUI;
class FWPhysicsObjectDesc;
class FWConfigurationManager;
class FWWebEventItemsManager;
class FWAssociationManager;
class FWTableViewManager;
class CmsShowNavigator;
class TSocket;

#include <thread>

#include "FireworksWeb/Core/interface/CmsShowMainBase.h"
#include "FireworksWeb/Core/interface/FWGeometry.h"
#include <sigc++/sigc++.h>

#include "TTimer.h"


class SignalTimer : public TTimer {
public:
   Bool_t Notify() override {
      timeout_();
      return true;
   }
   sigc::signal<void()> timeout_;
};

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

   void autoLoadNewEvent();
   void setupAutoLoad(float);


   const fwlite::Event* getCurrentEvent() const;
   const char* getFrameTitle() const;

   FWLiteJobMetadataManager *getMetadataManager() { return m_metadataManager; }
   const FWWebEventItemsManager *getEventItemsManager() const { return m_itemsManager; }
   void addFW2Item(bool, FWPhysicsObjectDesc &);

   void setConfigFilename(const std::string &f) { m_configFileName = f; };
   void doExit();
   void quit();

   bool getVersionCheck() const { return !m_noVersionCheck; }
   bool getGlobalTagCheck() const { return m_globalTagCheck; }

   const FWConfigurationManager* getConfigurationManager() {return m_configurationManager;}

   bool isStandalone() const { return m_standalone; }
   bool isPlaying() const { return m_autoplay; }

   bool isOpendataMode() const { return m_opendata; }

   // live
   void setupSocket(unsigned int iSocket);
   void notified(TSocket*);
   void setGUICtrlStates();
   //void startAutoLoadTimer();
   //void stopAutoLoadTimer();

   void do_autoplay();
   void autoplay_scheduler();
   void do_set_playdelay(float);
   void do_set_autoplay(bool);
   int  appendFile_thr(std::string);
   void appendFileFromMIR(const std::string&);

private:
   ROOT::Experimental::REveScene *m_collections{nullptr};
   FW2GUI *m_gui{nullptr};
   std::unique_ptr<CmsShowNavigator> m_navigator;
   std::unique_ptr<fireworks::Context> m_context;
   std::string m_configFileName;
   FWItemAccessorFactory *m_accessorFactory{nullptr};

   FW2EveManager *m_eveMng{nullptr};
   FWLiteJobMetadataManager *m_metadataManager{nullptr};
   FWWebEventItemsManager *m_itemsManager{nullptr};
   FWAssociationManager *m_associationManager{nullptr};
   FWConfigurationManager *m_configurationManager{nullptr};
   FWTableViewManager *m_tableManager{nullptr};
   Long64_t m_eventId;

   FWGeometry m_geom;
   std::string m_geometryFilename;

   bool m_standalone {true};
   bool m_globalTagCheck{true};
   // bool m_loadedAnyInputFile {false};
   std::atomic<bool> m_loadedAnyInputFile;

   std::vector<std::string> m_inputFiles;
   bool m_noVersionCheck{false};
   const TFile *m_openFile{nullptr};

   void draw_event();

   void setupConfiguration();
   void fileChangedSlot(const TFile *file);
   void eventChangedSlot();

   void setPlayLoop();
   void checkPosition();


   // autoplay
   std::thread *m_timerThread{nullptr};
   std::chrono::duration<double> m_deltaTime{1};
   std::mutex m_autoplay_mutex;
   std::condition_variable m_autoplay_cndvar;
   bool m_autoplay{false};
   bool m_end_autoplay_thread{false};

   
   // live options
   void setLiveMode(const std::string& lastFilePath);
   // void checkLiveMode();
   bool                         m_live{false};
   MyMon*                        m_monitor{nullptr};
   // std::unique_ptr<SignalTimer>   m_liveTimer;
   int                          m_liveTimeout{60000};
   UInt_t                       m_lastXEventSerial{0};
   std::string                  m_lastLiveAppend;

   // cms opendata mode
   bool m_opendata{false};

// std::thread* fTimerThread{nullptr};
// void liveTimer_thr();
  std::thread* m_appendFileThread{nullptr};
};

#endif
