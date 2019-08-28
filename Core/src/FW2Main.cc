#include <sstream>
#include <cstring>
#include <boost/program_options.hpp>

#include  "Fireworks2/Core/interface/FW2Main.h"
#include  "Fireworks2/Core/interface/Context.h"
#include  "Fireworks2/Core/interface/FWGeometry.h"
#include  "Fireworks2/Core/interface/FWMagField.h"
#include "Fireworks2/Core/interface/FWProxyBuilderFactory.h"

#include "DataFormats/FWLite/interface/Event.h"
#include "TROOT.h"
#include "TSystem.h"
#include "TEnv.h"

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
#include "Fireworks2/Core/interface/FWLiteJobMetadataManager.h"
#include "Fireworks2/Core/interface/FWLiteJobMetadataUpdateRequest.h"
#include "Fireworks2/Core/interface/fwLog.h"


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

using namespace ROOT::Experimental;

FW2Main::FW2Main(int argc, char *argv[])
{

   std::string descString(argv[0]);
   descString += " [options] <data file>\nGeneral";
   
   namespace po = boost::program_options;
   po::options_description desc(descString);
   desc.add_options()
      (kInputFilesCommandOpt, po::value< std::vector<std::string> >(),   "Input root files")
      //(kConfigFileCommandOpt, po::value<std::string>(),   "Include configuration file")
      //(kNoConfigFileCommandOpt,                           "Empty configuration")
      // (kNoVersionCheck,                                   "No file version check")
      (kPortCommandOpt, po::value<unsigned int>(),        "Listen to port for new data files to open")
      (kLogLevelCommandOpt, po::value<unsigned int>(),    "Set log level starting from 0 to 4 : kDebug, kInfo, kWarning, kError")
      (kEveCommandOpt,                                    "Eve plain interface")
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

   if (m_inputFiles.empty()) {
      fwLog(fwlog::kInfo) << "No data file given." << std::endl;
      exit(0);
   }
   else if (m_inputFiles.size() == 1)
      fwLog(fwlog::kInfo) << "Input " << m_inputFiles.front() << std::endl;
   else
      fwLog(fwlog::kInfo) << m_inputFiles.size() << " input files; first: " << m_inputFiles.front() << ", last: " << m_inputFiles.back() << std::endl;

   //______________________________________________________________________________

  
 
   edmplugin::PluginManager::configure(edmplugin::standard::config());

   // export to environment webgui settings
   gEnv->SetValue("WebGui.HttpMaxAge", 90000000);
   gEnv->SetValue("WebEve.DisableShow", 1);
   gEnv->SetValue("WebGui.SenderThrds", "yes");
      
   REX::REveManager::Create();
   const char* mypath =  Form("%s/src/Fireworks2/Core/ui5/",gSystem->Getenv("CMSSW_BASE"));
   printf("--- mypath ------ [%s] \n", mypath);

   if(vm.count(kEveCommandOpt)) {
      std::cout << "Eve debug GUI" <<std::endl;
   }
   else {
      ROOT::Experimental::gEve->AddLocation("mydir/",  mypath);
      ROOT::Experimental::gEve->SetDefaultHtmlPage("file:mydir/xxx.html");
   }
   ROOT::Experimental::gEve->SetClientVersion("00.01");

   auto geom = new FWGeometry();
   geom->loadMap("cmsGeom10.root");

   auto context = new fireworks::Context();
   context->initEveElements();
   context->setGeom(geom);
   context->getField()->checkFieldInfo(m_event);


   std::string fname = m_inputFiles.front().c_str();
   printf("---------------- %s \n", fname.c_str());
   m_file = TFile::Open(fname.c_str());
   m_event_tree = dynamic_cast<TTree*>(m_file->Get("Events"));
   m_event = 0;
   try
   {
      printf("---------------------------------------------------- STAGE 2\n");
      m_event = new fwlite::Event(m_file);
   }
   catch (const cms::Exception& iE)
   {
      printf("can't create a fwlite::Event\n");
      std::cerr << iE.what() <<std::endl;
      throw;
   }

   printf("---------------------------------------------------- STAGE 3\n");
   
   m_collections =  REX::gEve->SpawnNewScene("Collections","Collections");
   
   m_eveMng = new FW2EveManager();
   m_eveMng->setTableCollection("Tracks"); // temorary here, should be in collection

   m_eventId = 0;
   m_eventMng = new FW2EventManager();
   m_eventMng->SetName("EventManager");
   REX::gEve->GetWorld()->AddElement(m_eventMng);
   REX::gEve->GetWorld()->AddCommand("NextEvent", "sap-icon://step", m_eventMng, "NextEvent()");
   m_eventMng->setHandlerFunc([=] () { this->nextEvent();});

   // get ready for add collections 
   m_metadataManager = new FWLiteJobMetadataManager();
   m_metadataManager->initReps(m_eveMng->supportedTypesAndRepresentations());
   m_metadataManager->update(new FWLiteJobMetadataUpdateRequest(m_event, m_file));

   printf("---------------------------------------------------- STAGE 4\n");
   addTestItems();
   goto_event(1);
}

FW2Main::~FW2Main()
{
   delete m_event;
   delete m_file;
}

void FW2Main::nextEvent()
{
   goto_event(m_eventId);
      
}

void FW2Main::goto_event(Long64_t tid)
{  
   tid++;
   // AMT m_event->atEnd() can't be used
   if (tid == m_event->size()) {
      tid = 0;
   }

   m_eventId = tid;
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
   {
      FWDisplayProperties dp = FWDisplayProperties::defaultProperties;
      dp.setColor(kRed);
      FWPhysicsObjectDesc desc("Electrons",  TClass::GetClass("std::vector<reco::GsfElectron>"), "Muons", dp, "muons" );
      FWEventItem* item = new FWEventItem(m_accessorFactory->accessorFor(desc.type()), desc);
      m_items.push_back(item);
      m_eveMng->newItem(item);
   }
   {
      FWDisplayProperties dp = FWDisplayProperties::defaultProperties;
      dp.setColor(kRed);
      FWPhysicsObjectDesc desc("MET",  TClass::GetClass("std::vector<reco::PFMET>"), "MET", dp, "pfMet" );
      FWEventItem* item = new FWEventItem(m_accessorFactory->accessorFor(desc.type()), desc);
      m_items.push_back(item);
      m_eveMng->newItem(item);
   }
   {
      FWDisplayProperties dp = FWDisplayProperties::defaultProperties;
      dp.setColor(kYellow);
      FWPhysicsObjectDesc desc("Jets",  TClass::GetClass("std::vector<reco::CaloJet>"), "Jets", dp, "ak4CaloJets" );
      FWEventItem* item = new FWEventItem(m_accessorFactory->accessorFor(desc.type()), desc);
      m_items.push_back(item);
      m_eveMng->newItem(item);
   }
   {
      FWDisplayProperties dp = FWDisplayProperties::defaultProperties;
      dp.setColor(kRed);
      FWPhysicsObjectDesc desc("Muons",  TClass::GetClass("std::vector<reco::Muon>"), "Muons", dp, "muons" );
      FWEventItem* item = new FWEventItem(m_accessorFactory->accessorFor(desc.type()), desc);
      m_items.push_back(item);
      m_eveMng->newItem(item);
   }
   {
      FWDisplayProperties dp = FWDisplayProperties::defaultProperties;
      dp.setColor(kBlue);
      FWPhysicsObjectDesc desc("CSC-segments",  TClass::GetClass("CSCSegmentCollection"), "CSC-segments", dp, "cscSegments" );
      FWEventItem* item = new FWEventItem(m_accessorFactory->accessorFor(desc.type()), desc);
      m_items.push_back(item);
      m_eveMng->newItem(item);
   }
   {
      FWDisplayProperties dp = FWDisplayProperties::defaultProperties;
      dp.setColor(kGreen +2);
      FWPhysicsObjectDesc desc("Tracks",  TClass::GetClass("std::vector<reco::Track>"), "Tracks", dp, "generalTracks", "", "", "i.pt() > 1");
      FWEventItem* item = new FWEventItem(m_accessorFactory->accessorFor(desc.type()), desc);
      m_items.push_back(item);
      m_eveMng->newItem(item);
   }

}
