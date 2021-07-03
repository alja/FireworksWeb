#include <sstream>
#include <cstring>
#include <functional>

#include <boost/program_options.hpp>

#include "TROOT.h"
#include "TSystem.h"
#include "TEnv.h"
#include "TFile.h"

#include "ROOT/REveDataProxyBuilderBase.hxx"
#include "ROOT/REveElement.hxx"
#include "ROOT/REveManager.hxx"
#include "ROOT/REveScene.hxx"
#include "ROOT/RWebWindow.hxx"

#include "FWCore/PluginManager/interface/PluginFactory.h"
#include "FWCore/Reflection/interface/ObjectWithDict.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/PluginManager/interface/PluginManager.h"
#include "FWCore/PluginManager/interface/standard.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "DataFormats/CSCRecHit/interface/CSCSegmentCollection.h"
#include "DataFormats/FWLite/interface/Event.h"


#include  "FireworksWeb/Core/interface/FW2Main.h"
#include  "FireworksWeb/Core/interface/Context.h"
#include  "FireworksWeb/Core/interface/CmsShowNavigator.h"
#include  "FireworksWeb/Core/interface/FWGeometry.h"
#include  "FireworksWeb/Core/interface/FWMagField.h"
#include  "FireworksWeb/Core/interface/FWBeamSpot.h"
//#include "FireworksWeb/Core/interface/FWProxyBuilderFactory.h"
#include "FireworksWeb/Core/interface/FW2EveManager.h"
#include "FireworksWeb/Core/interface/FWSimpleRepresentationChecker.h"
#include "FireworksWeb/Core/interface/FWDisplayProperties.h"
#include "FireworksWeb/Core/interface/FWEventItem.h"
#include "FireworksWeb/Core/interface/FWItemAccessorBase.h"
#include "FireworksWeb/Core/interface/FWItemAccessorFactory.h"
#include "FireworksWeb/Core/interface/FWPhysicsObjectDesc.h"
#include "FireworksWeb/Core/interface/FWLiteJobMetadataManager.h"
#include "FireworksWeb/Core/interface/FWLiteJobMetadataUpdateRequest.h"
#include "FireworksWeb/Core/interface/FWConfigurationManager.h"
#include "FireworksWeb/Core/interface/FWEventItemsManager.h"
#include "FireworksWeb/Core/interface/FWTableViewManager.h"
#include "FireworksWeb/Core/interface/FW2GUI.h"
#include "FireworksWeb/Core/interface/fwLog.h"
#include "FireworksWeb/Core/src/SimpleSAXParser.h"
#include "FireworksWeb/Core/interface/FWWebGUIEventFilter.h"

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
static const char* const kRootInteractiveCommandOpt = "root-interactive,r";
static const char* const kChainCommandOpt = "chain";

using namespace ROOT::Experimental;

FW2Main::FW2Main():
   m_navigator(new CmsShowNavigator(*this)),
   m_context(new fireworks::Context()),
   m_accessorFactory(nullptr),
   
   m_eveMng(nullptr),
   m_metadataManager(nullptr),
   m_itemsManager(nullptr),
   m_configurationManager(nullptr),
   m_tableManager(nullptr)
{
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
   gEnv->SetValue("WebEve.DisableShow", 1);
   gEnv->SetValue("WebGui.SenderThrds", "yes");
      
   REveManager::Create();
   
   ROOT::Experimental::gEve->GetWebWindow()->SetClientVersion("00.06");

   m_context->initEveElements();
   m_context->setGeom(&m_geom);

   m_collections =  gEve->SpawnNewScene("Collections","Collections");
   
   m_itemsManager = new FWEventItemsManager;
   m_tableManager = new FWTableViewManager;
   m_eveMng = new FW2EveManager(m_tableManager);

   m_gui = new FW2GUI(this);
   m_gui->SetName("FW2GUI");
   gEve->GetWorld()->AddElement(m_gui);
   m_gui->AddElement(m_navigator->getGUIFilter());

   // get ready for add collections 
   m_metadataManager = new FWLiteJobMetadataManager();
   m_itemsManager->newItem_.connect(std::bind(&FW2EveManager::newItem, m_eveMng, std::placeholders::_1) );                                             
   m_configurationManager = new FWConfigurationManager();
   m_configurationManager->add("EventItems",m_itemsManager);
   m_configurationManager->add("Tables",m_tableManager);
   m_configurationManager->add("Tables",m_tableManager);
   m_configurationManager->add("EventNavigator", m_navigator.get());
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
      (kLogLevelCommandOpt, po::value<unsigned int>(),    "Set log level starting from 0 to 4 : kDebug, kInfo, kWarning, kError")
      (kEveCommandOpt,                                    "Eve plain interface")
      (kRootInteractiveCommandOpt,                        "Enable root prompt")
      (kChainCommandOpt,
      po::value<unsigned int>(),
      "Chain up to a given number of recently open files. Default is 1 - no chain")
      (kHelpCommandOpt,                                   "Display help message");


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
      std::cout << desc <<std::endl;
      exit(0); 
   }
   
   if(vm.count(kHelpOpt)) {
      std::cout << desc <<std::endl;
      exit(0);
   }
      
   if (vm.count(kPortCommandOpt)) {
      auto portNum = vm[kPortCommandOpt].as<unsigned int>();
      gEnv->SetValue("WebGui.HttpPort", (int)portNum);

   }
   if (vm.count(kLogLevelCommandOpt)) {
      fwlog::LogLevel level = (fwlog::LogLevel)(vm[kLogLevelCommandOpt].as<unsigned int>());
      fwlog::setPresentLogLevel(level);
   }
   
   
   // input file
   if (vm.count(kInputFilesOpt)) {
      m_inputFiles = vm[kInputFilesOpt].as<std::vector<std::string> >();
   }

   if (vm.count(kNoVersionCheck))
   {
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
      TString t = ino.c_str();
      const char* whereConfig = gSystem->FindFile(TROOT::GetMacroPath(), t, kReadPermission);
      if (!whereConfig)
      {
         fwLog(fwlog::kError) << "Specified configuration file does not exist. Quitting.\n";
         exit(1);
      }
      m_configFileName = whereConfig;
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
      catch (const std::runtime_error &iException)
      {
         fwLog(fwlog::kError) << "FW2ain::loadGeometry() caught exception: \n"
                              << m_geometryFilename << " " << iException.what() << std::endl;
         exit(0);
      }
   }
   if (m_inputFiles.empty()) {
      fwLog(fwlog::kInfo) << "No data file given." << std::endl;
      exit(0);
   }
   else if (m_inputFiles.size() == 1)
      fwLog(fwlog::kInfo) << "Input " << m_inputFiles.front() << std::endl;
   else
      fwLog(fwlog::kInfo) << m_inputFiles.size() << " input files; first: " << m_inputFiles.front() << ", last: " << m_inputFiles.back() << std::endl;

   
   // AMT ... the code below could be put in a separate function
   edmplugin::PluginManager::configure(edmplugin::standard::config());
   m_eveMng->createScenesAndViews();
   m_eveMng->initTypeToBuilder();

   m_metadataManager->initReps(m_eveMng->supportedTypesAndRepresentations());
   
   setupDataHandling();

   gEve->GetWorld()->AddCommand("Quit", "sap-icon://log", m_gui, "terminate()");
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
      if (!m_navigator->appendFile(fname, false, false))
      {
         std::cerr << "CANT OPEN FILE \n";
      }
      else
      {
         m_loadedAnyInputFile = true;
      }
   }

   if (m_loadedAnyInputFile)
   {
      m_navigator->firstEvent();
   }
   
   setupConfiguration();
   draw_event();
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
   {
      m_navigator->previousEvent();
      draw_event();
   }
}

void FW2Main::goToRunEvent(int run, int lumi, int event)
{
   m_navigator->goToRunEvent(edm::RunNumber_t(run), edm::LuminosityBlockNumber_t(lumi), edm::EventNumber_t(event));
   draw_event();     
}

void FW2Main::draw_event()
{  
  // printf("FW2Main::draw_event BEGIN\n");
   m_eveMng->beginEvent();
   m_itemsManager->newEvent(m_navigator->getCurrentEvent());
   
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

void FW2Main::addFW2Item(FWPhysicsObjectDesc& desc){
   std::string name = desc.purpose() + std::to_string(m_itemsManager->getNumItems()) + "_" + desc.moduleLabel();
   desc.setName(name);
   FWEventItem* item = m_itemsManager->add(desc);//m_accessorFactory->accessorFor(desc.type()), desc);
   m_eveMng->newItem(item);
   m_eveMng->beginEvent();
   item->setEvent(m_navigator->getCurrentEvent());
   m_eveMng->endEvent();
}

void
FW2Main::setupConfiguration()
{
   try
   { 
      if (m_configFileName.empty())
      {
         m_configFileName = m_configurationManager->guessAndReadFromFile(m_metadataManager);
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
   if (m_geometryFilename.empty())
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

//-_________________________________________________________________________
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
