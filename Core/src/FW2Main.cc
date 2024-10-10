


#include <sstream>
#include <cstring>
#include <functional>

#include <boost/program_options.hpp>

#include "TROOT.h"
#include "TSystem.h"
#include "TEnv.h"
#include "TFile.h"
#include "TMonitor.h"
#include "TServerSocket.h"

#include "ROOT/REveDataProxyBuilderBase.hxx"
#include "ROOT/REveElement.hxx"
#include "ROOT/REveManager.hxx"
#include "ROOT/REveScene.hxx"
#include "ROOT/RWebWindow.hxx"
#include "ROOT/RWebWindowsManager.hxx"

#include "FWCore/PluginManager/interface/PluginFactory.h"
#include "FWCore/Reflection/interface/ObjectWithDict.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/PluginManager/interface/PluginManager.h"
#include "FWCore/PluginManager/interface/standard.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "DataFormats/CSCRecHit/interface/CSCSegmentCollection.h"
#include "DataFormats/FWLite/interface/Event.h"


#include  "FireworksWeb/Core/interface/FW2Main.h"
#include  "FireworksWeb/Core/interface/fwPaths.h"
#include  "FireworksWeb/Core/interface/Context.h"
#include  "FireworksWeb/Core/interface/FWViewEnergyScale.h"
#include  "FireworksWeb/Core/interface/CmsShowNavigator.h"
#include  "FireworksWeb/Core/interface/FWGeometry.h"
#include  "FireworksWeb/Core/interface/FWMagField.h"
#include  "FireworksWeb/Core/interface/FWBeamSpot.h"
//#include "FireworksWeb/Core/interface/FWProxyBuilderFactory.h"
#include "FireworksWeb/Core/interface/FW2EveManager.h"
#include "FireworksWeb/Core/interface/FWSimpleRepresentationChecker.h"
#include "FireworksWeb/Core/interface/FWDisplayProperties.h"
#include "FireworksWeb/Core/interface/FWWebEventItem.h"
#include "FireworksWeb/Core/interface/FWItemAccessorBase.h"
#include "FireworksWeb/Core/interface/FWItemAccessorFactory.h"
#include "FireworksWeb/Core/interface/FWPhysicsObjectDesc.h"
#include "FireworksWeb/Core/interface/FWLiteJobMetadataManager.h"
#include "FireworksWeb/Core/interface/FWLiteJobMetadataUpdateRequest.h"
#include "FireworksWeb/Core/interface/FWConfigurationManager.h"
#include "FireworksWeb/Core/interface/FWWebEventItemsManager.h"
#include "FireworksWeb/Core/interface/FWAssociationManager.h"
#include "FireworksWeb/Core/interface/FWTableViewManager.h"
#include "FireworksWeb/Core/interface/FW2GUI.h"
#include "FireworksWeb/Core/interface/fwLog.h"
#include "FireworksWeb/Core/interface/SimpleSAXParser.h"
#include "FireworksWeb/Core/interface/FWWebGUIEventFilter.h"
#include "FireworksWeb/Core/interface/FWWebInvMassDialog.h"

static const char* const kInputFilesOpt        = "input-files";
static const char* const kInputFilesCommandOpt = "input-files,i";
static const char* const kConfigFileOpt        = "config-file";
static const char* const kConfigFileCommandOpt = "config-file,c";
static const char* const kGeomFileOpt          = "geom-file";
static const char* const kGeomFileCommandOpt   = "geom-file,g";
static const char* const kNoConfigFileOpt      = "noconfig";
static const char* const kNoConfigFileCommandOpt = "noconfig,n";
static const char* const kHelpOpt        = "help";
static const char* const kHelpCommandOpt = "help,h";
static const char* const kEveCommandOpt = "eve";
static const char* const kNoVersionCheck   = "no-version-check";
static const char* const kLogLevelCommandOpt   = "log";
static const char* const kPortCommandOpt = "port";
static const char* const kViewCommandOpt = "view";
static const char* const kRootInteractiveCommandOpt = "root-interactive,r";
static const char* const kChainCommandOpt = "chain";
static const char* const kLiveCommandOpt  = "live";
static const char* const kNewFilePortCommandOpt = "nc-port";
static const char* const kPlayOpt              = "play";
static const char* const kPlayCommandOpt       = "play,p";
static const char* const kLoopOpt              = "loop";
static const char* const kLoopCommandOpt       = "loop";
static const char* const kOpendataCommandOpt       = "opendata";


using namespace ROOT::Experimental;

FW2Main::FW2Main(bool standalone):
   m_navigator(new CmsShowNavigator(*this)),
   m_context(new fireworks::Context(this)),
   m_liveTimer(new SignalTimer())
{ 
   m_standalone = standalone;
   m_deltaTime = std::chrono::milliseconds(1000);
   m_loadedAnyInputFile = ATOMIC_VAR_INIT(false);

   ROOT::EnableThreadSafety(); // ??? AMT
   
   std::string macPath(gSystem->Getenv("CMSSW_BASE"));
   macPath += "/src/FireworksWeb/Core/macros";
   const char* base = gSystem->Getenv("CMSSW_RELEASE_BASE");
   if(nullptr!=base) {
      macPath+=":";
      macPath +=base;
      macPath +="/src/FireworksWeb/Core/macros";
   }
   gROOT->SetMacroPath((std::string("./:")+macPath).c_str());

   //______________________________________________________________________________
   // export to environment webgui settings
   gEnv->SetValue("WebGui.HttpMaxAge", 3600);
   gEnv->SetValue("WebGui.SenderThrds", "yes");
   gEnv->SetValue("WebEve.DisableShow", 1);
   gEnv->SetValue("WebEve.GLViewer", "RCore");
      
   REveManager::Create();
   
   ROOT::Experimental::gEve->GetWebWindow()->SetClientVersion(fireworks::clientVersion());
   
   // Authentification settings
   ROOT::RWebWindowsManager::SetLoopbackMode(false);
   ROOT::Experimental::gEve->GetWebWindow()->SetRequireAuthKey(false); 

   ROOT::RWebWindowsManager::SetUseSessionKey(true);

   ROOT::Experimental::gEve->GetWebWindow()->SetMaxQueueLength(100000);

   m_context->initEveElements();
   m_context->setGeom(&m_geom);

   m_collections =  gEve->SpawnNewScene("Collections","Collections");
   
   m_associationManager = new FWAssociationManager;
   m_tableManager = new FWTableViewManager;
   m_eveMng = new FW2EveManager(m_tableManager);
   m_itemsManager = new FWWebEventItemsManager(m_eveMng);

   m_gui = new FW2GUI(this);
   m_gui->SetName("FW2GUI");
   gEve->GetWorld()->AddElement(m_gui);
   m_gui->AddElement(m_navigator->getGUIFilter());
   m_gui->AddElement(m_context->energyScale());
   m_gui->AddElement(new FWWebInvMassDialog());

   // get ready for add collections 
   m_metadataManager = new FWLiteJobMetadataManager();
   m_itemsManager->newItem_.connect(std::bind(&FW2EveManager::newItem, m_eveMng, std::placeholders::_1) );                                             
   m_configurationManager = new FWConfigurationManager();
   m_configurationManager->add("EventItems",m_itemsManager);
   m_configurationManager->add("Associations",m_associationManager);
   m_configurationManager->add("EventNavigator", m_navigator.get());
   m_configurationManager->add("Tables",m_tableManager);
   // at the moment scales are put directly since they are theonly settings
   m_configurationManager->add("CommonPreferences", m_context->energyScale());

   
   m_context->energyScale()->parameterChanged_.connect(std::bind(&FW2EveManager::globalEnergyScaleChanged, m_eveMng));
   m_context->energyScale()->backgroundChanged_.connect(std::bind(&FW2EveManager::globalBackgroundChanged, m_eveMng));
}

FW2Main::~FW2Main()
{
}

void FW2Main::parseArguments(int argc, char *argv[])
{  
   std::string descString(argv[0]);

   descString += " [options] <data file>\nGeneral";
   
   namespace po = boost::program_options;
   po::options_description desc(descString);
   desc.add_options()
      (kInputFilesCommandOpt, po::value< std::vector<std::string> >(),   "Input root files")
      (kConfigFileCommandOpt, po::value<std::string>(),   "Include configuration file")
      (kNoConfigFileCommandOpt,                           "Empty configuration")
      (kGeomFileCommandOpt, po::value<std::string>(),     "Reco geometry file. Default is autodetected")
      (kNoVersionCheck,                                   "No file version check")
      (kPortCommandOpt, po::value<unsigned int>(),        "Http server port")
      (kViewCommandOpt, po::value<std::string>(),         "View list. By the defult all views are visible 3D:RPhi:RhoZ:Table:TriggerTable")
      (kLogLevelCommandOpt, po::value<unsigned int>(),    "Set log level starting from 0 to 4 : kDebug, kInfo, kWarning, kError")
      (kEveCommandOpt,                                    "Eve plain interface")
      (kRootInteractiveCommandOpt,                        "Enable root prompt")
      (kOpendataCommandOpt,                               "Opendata mode")
      (kHelpCommandOpt,                                   "Display help message");


   po::options_description livedesc("Live Event Display");
    livedesc.add_options()
      (kPlayCommandOpt, po::value<float>(),               "Start in play mode with given interval between events in seconds")
      (kNewFilePortCommandOpt, po::value<unsigned int>(),        "Listen to port for new data files to open")
      (kLoopCommandOpt,                                   "Loop events in play mode")
      (kChainCommandOpt, po::value<unsigned int>(),       "Chain up to a given number of recently open files. Default is 1 - no chain")
      (kLiveCommandOpt,                                   "Enforce playback mode if a user is not using display");
   

   desc.add(livedesc);

   po::positional_options_description p;
   p.add(kInputFilesOpt, -1);
   
   int newArgc = argc;
   char **newArgv = argv;
   po::variables_map vm;


   try {
      po::store(po::command_line_parser(newArgc, newArgv).
                options(desc).positional(p).run(), vm);

      po::notify(vm);
   }
   catch (const std::exception& e)
   {
      // Return with exit status 0 to avoid generating crash reports

      fwLog(fwlog::kError) <<  e.what() << std::endl;
      std::cout << desc << std::endl;
      throw; 
   }
   
   if (vm.count(kHelpOpt)) {
      std::cout << desc <<std::endl;
      exit(0);
   }

   if (vm.count(kOpendataCommandOpt)) {
      printf("CMS Opendata is enabled. \n");
      m_opendata = true;
   }
      
   if (vm.count(kPortCommandOpt)) {
      auto portNum = vm[kPortCommandOpt].as<unsigned int>();
      gEnv->SetValue("WebGui.HttpPort", (int)portNum);
   }
  
   if (vm.count(kLoopOpt))
      setPlayLoop();

   if(vm.count(kChainCommandOpt)) {
      m_navigator->setMaxNumberOfFilesToChain(vm[kChainCommandOpt].as<unsigned int>());
   }


   std::string tmpViewOption;
   if (vm.count(kViewCommandOpt)) {
      tmpViewOption = vm[kViewCommandOpt].as<std::string>();
   }


   if (vm.count(kLogLevelCommandOpt)) {
      fwlog::LogLevel level = (fwlog::LogLevel)(vm[kLogLevelCommandOpt].as<unsigned int>());
      fwlog::setPresentLogLevel(level);
   }
   
   
   // input file
   if (vm.count(kInputFilesOpt)) {
      m_inputFiles = vm[kInputFilesOpt].as<std::vector<std::string> >();
   }

   if (vm.count(kNoVersionCheck)) {
      m_noVersionCheck = true;
   }

   // ROOT client GUI
   if (vm.count(kEveCommandOpt))
   {
      std::cout << "Eve debug GUI" << std::endl;
   }
   else {
      const char* mypath =  Form("%s/src/FireworksWeb/Core/ui5/",gSystem->Getenv("CMSSW_BASE"));
      // printf("--- mypath ------ [%s] \n", mypath);
      std::string fp = "fireworks-" + ROOT::Experimental::gEve->GetWebWindow()->GetClientVersion()+"/";
      ROOT::Experimental::gEve->AddLocation(fp,  mypath);
      std::string dp = (Form("file:%s/fireworks.html", fp.c_str()));
      ROOT::Experimental::gEve->SetDefaultHtmlPage(dp);

      fwLog(fwlog::kDebug) << "Default html page " << dp << std::endl;
   }

   if (vm.count(kChainCommandOpt))
   {
      m_navigator->setMaxNumberOfFilesToChain(vm[kChainCommandOpt].as<unsigned int>());
   }

   // configuration file
   if (vm.count(kConfigFileOpt))
   {
      std::string ino = vm[kConfigFileOpt].as<std::string>();
      /*
      TString t = ino.c_str();
      
      const char* whereConfig = gSystem->FindFile(TROOT::GetMacroPath(), t, kReadPermission);
      if (!whereConfig)
      {
         throw std::runtime_error("Specified configuration file does not exist.");
      }*/
      m_configFileName = ino;
      fwLog(fwlog::kInfo) << "Config "  <<  m_configFileName << std::endl;
   }
   else
   {
      if (vm.count(kNoConfigFileOpt)) {
         fwLog(fwlog::kInfo) << "No configuration is loaded.\n";
         m_configurationManager->setIgnore();
      }
   }

   if (vm.count(kGeomFileOpt))
   {
      m_geometryFilename = vm[kGeomFileOpt].as<std::string>();
      fwLog(fwlog::kInfo) << "Geometry file " << m_geometryFilename << "\n";

      try
      {
         m_geom.loadMap(m_geometryFilename.c_str());
      }
      catch (const std::runtime_error &exc)
      {
         std::ostringstream ext_exc("FW2Main::loadGeometry() caught exception:\n   ");
         ext_exc << m_geometryFilename << " " << exc.what();
         throw std::runtime_error(ext_exc.str());
      }
      m_globalTagCheck = false;
   }
   if (m_inputFiles.empty()) {
      ///throw std::runtime_error("No data file given.");
      std::cout << "no input file \n";
   }
   else if (m_inputFiles.size() == 1)
      fwLog(fwlog::kInfo) << "Input " << m_inputFiles.front() << std::endl;
   else
      fwLog(fwlog::kInfo) << m_inputFiles.size() << " input files; first: " << m_inputFiles.front() << ", last: " << m_inputFiles.back() << std::endl;

   // AMT ... the code below could be put in a separate function
   edmplugin::PluginManager::configure(edmplugin::standard::config());
   m_eveMng->createScenesAndViews(tmpViewOption);
   m_eveMng->initTypeToBuilder();

   m_metadataManager->initReps(m_eveMng->supportedTypesAndRepresentations());
   m_metadataManager->initAssociationTypes(m_associationManager);
   
   // load data and configuration
   setupDataHandling();

   if (vm.count(kNewFilePortCommandOpt)) { 	 
      setupSocket(vm[kNewFilePortCommandOpt].as<unsigned int>());
   }

   if (vm.count(kPlayOpt))
   {
      setupAutoLoad(vm[kPlayOpt].as<float>());
   }

   if (vm.count(kLiveCommandOpt))
   {
      setLiveMode();
   }
}

void FW2Main::setupDataHandling()
{
   m_navigator->fileChanged_.connect(std::bind(&FW2Main::fileChangedSlot, this, std::placeholders::_1));
   m_navigator->newEvent_.connect(std::bind(&FW2Main::eventChangedSlot, this));

   m_navigator->postFiltering_.connect(std::bind(&FW2Main::postFiltering, this, std::placeholders::_1));

   for (unsigned int ii = 0; ii < m_inputFiles.size(); ++ii)
   {
      const std::string &fname = m_inputFiles[ii];
      if (fname.empty())
         continue;

      if ( ! m_navigator->appendFile(fname, false, false))
      {
         std::string es("Error opening input file ");
         throw std::runtime_error(es + fname);
      }
      m_loadedAnyInputFile = true;
   }

   if (m_loadedAnyInputFile)
   {
      m_navigator->firstEvent();
      setupConfiguration();
      draw_event();
   }
   else {
      setupConfiguration();
   }
}

void FW2Main::nextEvent()
{
   if (m_navigator->isLastEvent())
   {
      if (getLoop())
      {
         m_navigator->firstEvent();
         draw_event();
      }
   }
   else
   {
      m_navigator->nextEvent();
      draw_event();
   }
}

void FW2Main::previousEvent()
{
   if (m_navigator->isFirstEvent())
   {
      if (getLoop())
      {
         m_navigator->lastEvent();
         draw_event();
      }
   }
   else
   {
      m_navigator->previousEvent();
      draw_event();
   }
}

void FW2Main::firstEvent()
{
   m_navigator->firstEvent();
   draw_event();
}

void FW2Main::lastEvent()
{
   m_navigator->lastEvent();
   draw_event();
}

void FW2Main::goToRunEvent(int run, int lumi, int event)
{
   m_navigator->goToRunEvent(edm::RunNumber_t(run), edm::LuminosityBlockNumber_t(lumi), edm::EventNumber_t(event));
   draw_event();     
}

void FW2Main::draw_event()
{
   printf("FW2Main::draw_event BEGIN\n");
   m_eveMng->beginEvent();
   m_itemsManager->newEvent(m_navigator->getCurrentEvent());

   if (m_context->getField()->getSource() != FWMagField::kUser)
   {
      m_context->getField()->checkFieldInfo(m_navigator->getCurrentEvent());
   }
   m_context->getBeamSpot()->checkBeamSpot(m_navigator->getCurrentEvent());
   m_eveMng->endEvent();
   m_gui->StampObjProps();
   // printf("FW2Main::draw_event END\n");
}

void FW2Main::postFiltering(bool doDraw)
{
   m_navigator->getGUIFilter()->StampObjProps();
   if (doDraw) draw_event();     
}

void FW2Main::addFW2Item(bool isEDM, FWPhysicsObjectDesc &desc)
{
   m_eveMng->beginEvent();
   if (isEDM)
   {
      std::string name = desc.purpose() + std::to_string(m_itemsManager->getNumItems()) + "_" + desc.moduleLabel();
      desc.setName(name);
      FWWebEventItem *item = m_itemsManager->add(desc);

      std::stringstream ss;
      for (auto &t : item->GetItemList()->RefToolTipExpressions())
         ss << t->fTooltipFunction.GetFunctionExpressionString();
      gROOT->ProcessLine(ss.str().c_str());
      item->setEvent(m_navigator->getCurrentEvent());
   }
   else
   {
      m_associationManager->addAssociation(desc);
   }
   
   m_eveMng->endEvent();
}

void
FW2Main::setupConfiguration()
{
   try
   { 
      if (m_configFileName.empty())
      {
         m_configFileName = m_configurationManager->guessAndReadFromFile(m_metadataManager, m_opendata);
      }
      else
      {
         m_configurationManager->readFromFile(m_configFileName);
      }
   }
   catch (SimpleSAXParser::ParserError &e)
   {
      fwLog(fwlog::kError) <<"Unable to load configuration file '" 
                           << m_configFileName 
                           << "': " 
                           << e.error()
                           << std::endl;
      exit(1);
   }
   catch (std::runtime_error &e)
   {
      fwLog(fwlog::kError) <<"Unable to load configuration file '" 
                           << m_configFileName 
                           << "' which was specified on command line. Quitting." 
                           << std::endl;
      exit(1);
   }       
}

void FW2Main::fileChangedSlot(const TFile *file)
{
   m_openFile = file;
   if (m_context->getField()->getSource() == FWMagField::kNone)
   {
      m_context->getField()->resetFieldEstimate();
   }
   if (m_geometryFilename.empty() && (m_live == false))
   {
      std::string gt = m_navigator->getCurrentGlobalTag();
      m_geom.applyGlobalTag(gt);
   }
   m_metadataManager->update(new FWLiteJobMetadataUpdateRequest(getCurrentEvent(), m_openFile));
}

void FW2Main::eventChangedSlot()
{
   // AMT why need to updated collection list on new event ?
   //     this is already done in fileChanged slot
  // m_metadataManager->update(new FWLiteJobMetadataUpdateRequest(getCurrentEvent(), m_openFile));
}

const fwlite::Event* FW2Main::getCurrentEvent() const
{
  return dynamic_cast<const fwlite::Event*>(m_navigator->getCurrentEvent());
}

const char* FW2Main::getFrameTitle() const
{
   return m_navigator->frameTitle();
}

void FW2Main::setPlayLoop() {
   std::cout << "FW2Main::setPlayLoop() activated\n";
   m_loop = true;
}

//_________________________________________________________________________
// AMT why do we need  die timer ?
class DieTimer : public TTimer {
protected:
  FW2Main* fApp;

public:
  DieTimer(FW2Main* app) : TTimer(), fApp(app) { Start(0, kTRUE); }

  Bool_t Notify() override {
    TurnOff();
    fApp->doExit();
    delete this;
    return kFALSE;
  }
};

void FW2Main::quit() { new DieTimer(this); }

void FW2Main::doExit() {
  exit(0);
}

//______________________________________________________________________________
//______________________________________________________________________________
//________________________ NEW FIle notification through netcat  _______________
//______________________________________________________________________________

void
FW2Main::setupSocket(unsigned int iSocket)
{
   m_monitor = std::make_unique<TMonitor>(new TMonitor);
   TServerSocket* server = new TServerSocket(iSocket,kTRUE);
   if (server->GetErrorCode())
   {
      fwLog(fwlog::kError) << "FW2Main::setupSocket, can't create socket on port "<< iSocket << "." << std::endl;
      exit(0);
   }    
   m_monitor->Add(server);
   connectSocket();
}

//______________________________________________________________________________

void
FW2Main::connectSocket()
{
  m_monitor->Connect("Ready(TSocket*)","FW2Main",this,"notified(TSocket*)");
}

//______________________________________________________________________________

void
FW2Main::notified(TSocket* iSocket)
{
   TServerSocket* server = dynamic_cast<TServerSocket*> (iSocket);
   if (server)
   {
      TSocket* connection = server->Accept();
      if (connection)
      {
         m_monitor->Add(connection);
         // std::cout  << "received connection from "<<iSocket->GetInetAddress().GetHostName();
      }
   }
   else
   {
      char buffer[4096];
      memset(buffer, 0, sizeof(buffer));
      // std::cout << "--------- " << buffer << "\n";
      if (iSocket->RecvRaw(buffer, sizeof(buffer)) <= 0)
      {
         m_monitor->Remove(iSocket);
         fwLog(fwlog::kInfo) << "closing connection to " << iSocket->GetInetAddress().GetHostName() << "\n";
         delete iSocket;
         return;
      }

      REveManager::ChangeGuard ch;

      std::string fileName(buffer);
      std::string::size_type lastNonSpace = fileName.find_last_not_of(" \n\t");
      if (lastNonSpace != std::string::npos)
      {
         fileName.erase(lastNonSpace + 1);
      }

      // std::cout << "------ New file notified '" << fileName << "' \n";
      bool appended = m_navigator->appendFile(fileName, true, m_live);

      if (appended)
      {
         if (m_live && isPlaying())
         {
            m_navigator->activateNewFileOnNextEvent();
         }
         else if (!isPlaying())
         {
            checkPosition();
         }

         // bootstrap case: --port  and no input file
         if (!m_loadedAnyInputFile)
         {
            m_loadedAnyInputFile = true;
            m_CV.notify_all();
         }

         fwLog(fwlog::kInfo) << "New file registered '" << fileName << "'";
      }
      else
      {
         fwLog(fwlog::kError) << "New file NOT registered '" << fileName << "'";
      }
   }
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// ------------------------- AUTOPLAY -------------------------------------------
// -----------------------------------------------------------------------------
// called from parsing of argumetns
void FW2Main::setupAutoLoad(float x)
{
   m_gui->setPlayDelayInMiliseconds(x*1000);
   m_gui->setAutoplay(true);
}

// -----------------------------------------------------------------------------
// function called from autoplay_scheduler thread
void FW2Main::autoLoadNewEvent()
{
   // std::cout << "FW2Main::autoLoadNewEvent begin\n";
   if (!m_loadedAnyInputFile)
   {
      std::cout << "no data loaded !!! \n";
      return;
   }

   // std::cout << "FW2Main::autoLoadNewEvent wait REveManager::ChangeGuard \n";
   REveManager::ChangeGuard ch;
   bool reachedEnd = m_navigator->isLastEvent();

   if (m_loop && reachedEnd)
   {
      m_navigator->firstEvent();
      draw_event();
   }
   else if (!reachedEnd)
   {
      // std::cout << "FW2Main::autoLoadNewEvent do next event from \n";
      m_navigator->nextEvent();
      draw_event();
   }

  // std::cout << "FW2Main::autoLoadNewEvent end\n";
}

// -----------------------------------------------------------------------------
// Thread with function of timer
void FW2Main::autoplay_scheduler()
{
   pthread_setname_np(pthread_self(), "autoplay");
   while (true)
   {
      bool autoplay;
      {
         std::unique_lock<std::mutex> lock{m_mutex};
         if (!m_autoplay)
         {
            printf("exit thread pre wait\n");
            return;
         }
         if (m_CV.wait_for(lock, m_deltaTime) != std::cv_status::timeout)
         {
            printf("autoplay not timed out \n");
            if (!m_autoplay)
            {
               printf("exit thread post wait\n");
               return;
            }
            else
            {
               continue;
            }
         }
         autoplay = m_autoplay;
      }
      if (autoplay)
      {
         std::cout << "auto load called from hthread\n";
         autoLoadNewEvent();
      }
      else
      {
         return;
      }
   }
}

// -----------------------------------------------------------------------------
// Turn off/on autoloaf of events
// function called from MIR execution thread
void FW2Main::do_set_autoplay(bool x)
{
   fwLog(fwlog::kInfo) << "FW2Main::do_set_autoplay " << x << std::endl;
   static std::mutex autoplay_mutex;
   std::unique_lock<std::mutex> aplock{autoplay_mutex};
   {
      std::unique_lock<std::mutex> lock{m_mutex};

      m_autoplay = x;
      if (m_autoplay)
      {
         if (m_timerThread)
         {
            std::cout << "FW2Main::do_set_autoplay, kill thread\n";
            m_timerThread->join();
            delete m_timerThread;
            m_timerThread = nullptr;
         }
         std::cout << "FW2Main::do_set_autoplay call next event \n";
         if (m_loadedAnyInputFile)
            nextEvent();
         std::cout << "make auto play thread \n";
         m_timerThread = new std::thread{[this]
                                         { autoplay_scheduler(); }};
      }
      else
      {
         m_CV.notify_all();
      }
   }
}
// -----------------------------------------------------------------------------
// Change wait time between events
void FW2Main::do_set_playdelay(float x)
{
   printf("FW2Main::do_set_playfdelay %f\n", x);
   if (x < 1000)
   {
       fwLog(fwlog::kError) << "Can't set play delay less than 1s";
       x = 1000;
   }
   std::unique_lock<std::mutex> lock{m_mutex};
   m_deltaTime =  std::chrono::milliseconds(int(x));
   m_CV.notify_all();
}
//______________________________________________________________________________
//______________________________________________________________________________
//________________________ LIVE TIMER __________________________________________
//______________________________________________________________________________

void FW2Main::liveTimer_thr()
{
   pthread_setname_np(pthread_self(), "livetimer");
   REveServerStatus ss;
   gEve->GetServerStatus(ss);
   if (ss.fTLastMir == 0)
   {
      std::cout << "no connections the server yet\n";
      return;
   }
   std::time_t now = std::time(0);
   std::time_t dt = now - ss.fTLastMir;
   std::cout << "FW2Main::checkLiveMode  " << ss.fTLastMir << " now " << now << "   ====== delta ==== " << dt<<"\n";
   std::time_t maxd = 600; // in second untis
   if (dt > maxd)
   {
      std::cout << "more than " << maxd << " !!!!\n";
      REveManager::ChangeGuard ch;
      m_gui->setAutoplay(true);
   }
}

void FW2Main::setLiveMode() {
  m_live = true;
  m_liveTimer.reset(new SignalTimer());
  m_liveTimer->timeout_.connect(std::bind(&FW2Main::checkLiveMode, this));
  m_liveTimer->SetTime(m_liveTimeout);
  m_liveTimer->Reset();
  m_liveTimer->TurnOn();
}

void FW2Main::checkLiveMode()
{
   m_liveTimer->TurnOff();
   if (!isPlaying())
   {
      if (fTimerThread)
      {
         fTimerThread->join();
         delete fTimerThread;
         fTimerThread = nullptr;
      }
      fTimerThread = new std::thread{[this]
                                     { liveTimer_thr(); }};
   }
   m_liveTimer->SetTime((Long_t)(m_liveTimeout));
   m_liveTimer->Reset();
   m_liveTimer->TurnOn();
}

//______________________________________________________________________________
void 
FW2Main::checkPosition()
{
   if ((m_monitor.get() || getLoop() ) && isPlaying())
      return;
   
   m_gui->StampObjProps();
   std::cout << "checkPosition ???\n";
}
//______________________________________________________________________________

void FW2Main::setGUICtrlStates()
{
   FW2GUI::CtrlStates_t& s = m_gui->refCtrlStates();
   s.clear();

   if (!m_loadedAnyInputFile)
   {
      s.push_back("empty");
      return;
   }

   if (m_navigator->isFirstEvent())
   s.push_back("first");

   if (m_navigator->isLastEvent())
   s.push_back("last");
}
