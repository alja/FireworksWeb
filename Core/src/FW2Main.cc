#include <sstream>
#include <cstring>
#include <boost/program_options.hpp>
#include <boost/bind.hpp>


#include "TROOT.h"
#include "TSystem.h"
#include "TEnv.h"
#include "TTree.h"
#include "TFile.h"

#define protected public
#include <boost/bind.hpp>
#include "ROOT/REveDataProxyBuilderBase.hxx"
#include "ROOT/REveElement.hxx"
#include "ROOT/REveManager.hxx"
#include "ROOT/REveScene.hxx"
#include "ROOT/RWebWindow.hxx"


//#define protected protected

// system include files
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
#include  "FireworksWeb/Core/interface/FWGeometry.h"
#include  "FireworksWeb/Core/interface/FWMagField.h"
#include "FireworksWeb/Core/interface/FWProxyBuilderFactory.h"
#include "FireworksWeb/Core/interface/FW2EveManager.h"
#include "FireworksWeb/Core/interface/FWSimpleRepresentationChecker.h"
#include "FireworksWeb/Core/interface/FWDisplayProperties.h"
#include "FireworksWeb/Core/interface/FWPhysicsObjectDesc.h"
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

using namespace ROOT::Experimental;

FW2Main::FW2Main():
   m_file(nullptr),
   m_event_tree(nullptr),
   m_event(nullptr),
   m_collections(nullptr),
   m_gui(nullptr),

   m_accessorFactory(nullptr),
   
   m_eveMng(nullptr),
   m_metadataManager(nullptr),
   m_itemsManager(nullptr),
   m_configurationManager(nullptr),
   m_tableManager(nullptr),
   
   m_eventId(0)
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
      
   REX::REveManager::Create();
   
   ROOT::Experimental::gEve->GetWebWindow()->SetClientVersion("00.05");

   auto geom = new FWGeometry();
   geom->loadMap("cmsGeom10.root");

   auto context = new fireworks::Context();
   context->initEveElements();
   context->setGeom(geom);

   m_collections =  REX::gEve->SpawnNewScene("Collections","Collections");
   
   m_itemsManager = new FWEventItemsManager;
   m_tableManager = new FWTableViewManager;
   m_eveMng = new FW2EveManager(m_tableManager);

   m_gui = new FW2GUI(this);
   m_gui->SetName("FW2GUI");
   REX::gEve->GetWorld()->AddElement(m_gui);

   // get ready for add collections 
   m_metadataManager = new FWLiteJobMetadataManager();
   m_itemsManager->newItem_.connect(boost::bind(&FW2EveManager::newItem, m_eveMng, _1) );                                             
   m_configurationManager = new FWConfigurationManager();
   m_configurationManager->add("EventItems",m_itemsManager);
   m_configurationManager->add("Tables",m_tableManager);

   
}

FW2Main::~FW2Main()
{
   delete m_event;
   delete m_file;
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
      // (kNoVersionCheck,                                   "No file version check")
      (kPortCommandOpt, po::value<unsigned int>(),        "Listen to port for new data files to open")
      (kLogLevelCommandOpt, po::value<unsigned int>(),    "Set log level starting from 0 to 4 : kDebug, kInfo, kWarning, kError")
      (kEveCommandOpt,                                    "Eve plain interface")
      (kRootInteractiveCommandOpt,                        "Enable root prompt")
      (kHelpCommandOpt,                                   "Display help message");


   po::positional_options_description p;
   p.add(kInputFilesOpt, -1);
   
   int newArgc = argc;
   char **newArgv = argv;
   po::variables_map vm;

 
   try{ 
      po::store(po::command_line_parser(newArgc, newArgv).
                options(desc).positional(p).run(), vm);

      po::notify(vm);
   }
   catch ( const std::exception& e)
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
      
   if(vm.count(kPortCommandOpt)) {
      auto portNum = vm[kPortCommandOpt].as<unsigned int>();
      gEnv->SetValue("WebGui.HttpPort", (int)portNum);

   }
   if(vm.count(kLogLevelCommandOpt)) {
      fwlog::LogLevel level = (fwlog::LogLevel)(vm[kLogLevelCommandOpt].as<unsigned int>());
      fwlog::setPresentLogLevel(level);
   }
   
   
   // input file
   if (vm.count(kInputFilesOpt)) {
      m_inputFiles = vm[kInputFilesOpt].as<std::vector<std::string> >();
   }

   // ROOT client GUI
     if(vm.count(kEveCommandOpt)) {
      std::cout << "Eve debug GUI" <<std::endl;
   }
   else {
      const char* mypath =  Form("%s/src/FireworksWeb/Core/ui5/",gSystem->Getenv("CMSSW_BASE"));
       printf("--- mypath ------ [%s] \n", mypath);
      std::string fp = "fireworks-" + ROOT::Experimental::gEve->GetWebWindow()->GetClientVersion()+"/";
      //std::string fp = "fireworks/";
      ROOT::Experimental::gEve->AddLocation(fp,  mypath);
      std::string dp = (Form("file:%s/fireworks.html", fp.c_str()));
      ROOT::Experimental::gEve->SetDefaultHtmlPage(dp);
      
      fwLog(fwlog::kDebug) << "Default html page " << dp << std::endl;
   }
   // configuration file
   if (vm.count(kConfigFileOpt)) {
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
   } else {
      if (vm.count(kNoConfigFileOpt)) {
         fwLog(fwlog::kInfo) << "No configuration is loaded.\n";
         m_configurationManager->setIgnore();
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

   // !!!
   // !!!
   // AMT this should be somewhere else
   edmplugin::PluginManager::configure(edmplugin::standard::config());
   m_eveMng->createScenesAndViews();
   m_eveMng->initTypeToBuilder();
   loadInputFiles();

   printf("---------------------------------------------------- STAGE 3 init Eve\n");
   
   fireworks::Context::getInstance()->getField()->checkFieldInfo(m_event);
   m_metadataManager->initReps(m_eveMng->supportedTypesAndRepresentations());
   m_metadataManager->update(new FWLiteJobMetadataUpdateRequest(m_event, m_file));

   printf("---------------------------------------------------- STAGE 4 setup Firework mangers\n");
  

   setupConfiguration();

   REX::gEve->GetWorld()->AddCommand("Quit", "sap-icon://log", m_gui, "terminate()");
   
   goto_event(m_eventId);
}


void FW2Main::loadInputFiles()
{  
   std::string fname = m_inputFiles.front().c_str();
   printf("---------------- %s \n", fname.c_str());
   m_file = TFile::Open(fname.c_str());
   m_event_tree = dynamic_cast<TTree*>(m_file->Get("Events"));
   try
   {
      printf("---------------------------------------------------- STAGE 2 create fwlite::Event\n");
      m_event = new fwlite::Event(m_file);
   }
   catch (const cms::Exception& iE)
   {
      printf("can't create a fwlite::Event\n");
      std::cerr << iE.what() <<std::endl;
   }

 
}

void FW2Main::nextEvent()
{
   int tid = m_eventId + 1;
   
   // AMT m_event->atEnd() can't be used
   if (tid == m_event->size()) {
      tid = 0;
   }

   goto_event(tid);     
}


void FW2Main::previousEvent()
{
   int tid = m_eventId;
   if (tid == 0) {
      tid = m_event->size() -1;
   }
   else {
      tid  = tid -1;
   }
   goto_event(tid);
   
}

void FW2Main::goto_event(Long64_t tid)
{  
   m_eventId = tid;
   m_event->to(tid);
   m_event_tree->LoadTree(tid);

   m_eveMng->beginEvent();
   m_itemsManager->newEvent(m_event);
   
   m_eveMng->endEvent();
   m_gui->m_ecnt = tid;
   m_gui->StampObjProps();
}

void FW2Main::addFW2Item(FWPhysicsObjectDesc& desc){
   std::string name = desc.purpose() + std::to_string(m_itemsManager->getNumItems()) + "_" + desc.moduleLabel();
   desc.setName(name);
   FWEventItem* item = m_itemsManager->add(desc);//m_accessorFactory->accessorFor(desc.type()), desc);
   m_eveMng->newItem(item);
   m_eveMng->beginEvent();
   item->setEvent(m_event);
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
