
#include <fstream>

#include "FireworksWeb/Core/interface/FW2GUI.h"
#include "FireworksWeb/Core/interface/BuilderUtils.h"
#include "FireworksWeb/Core/interface/FWJobMetadataManager.h"
#include "FireworksWeb/Core/interface/FWLiteJobMetadataManager.h"
#include "FireworksWeb/Core/interface/FWDisplayProperties.h"
#include "FireworksWeb/Core/interface/FWPhysicsObjectDesc.h"
#include "FireworksWeb/Core/interface/fwLog.h"
#include "FireworksWeb/Core/interface/FWConfiguration.h"
#include "FireworksWeb/Core/interface/FWConfigurationManager.h"
//#include "FireworksWeb/Core/interface/CmsShowNavigator.h"
#include "DataFormats/FWLite/interface/Event.h"

#include "TFile.h"

#include "nlohmann/json.hpp"
using namespace ROOT::Experimental;

FW2GUI::FW2GUI() : m_main(0) {
   m_deltaTime = std::chrono::milliseconds(500);
}

FW2GUI::FW2GUI(FW2Main* m) : m_main(m) {
}

FW2GUI::~FW2GUI()
{
}


void FW2GUI::terminate()
{
   m_main->quit();
}

void
FW2GUI::NextEvent()
{
   m_main->nextEvent();
}

void
FW2GUI::PreviousEvent()
{
   m_main->previousEvent();
}

void
FW2GUI::FirstEvent()
{
   m_main->firstEvent();
}

void
FW2GUI::LastEvent()
{
   m_main->lastEvent();
}


void
FW2GUI::goToRunEvent(int run, int lumi, int event)
{
   // printf("gotot... %d %d %d \n", run, lumi, event);
   m_main->goToRunEvent(edm::RunNumber_t(run), edm::LuminosityBlockNumber_t(lumi), edm::EventNumber_t(event));
}

void FW2GUI::autoplay_scheduler()
{
   while (true)
   {
      bool autoplay;
      {
         std::unique_lock<std::mutex> lock{m_mutex};
         if (!m_autoplay)
         {
            // printf("exit thread pre wait\n");
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
         REveManager::ChangeGuard ch;
         NextEvent();
      }
      else
      {
         return;
      }
   }
}

void FW2GUI::autoplay(bool x)
{
   fwLog(fwlog::kInfo) << "Set autoplay " << x << std::endl;
   static std::mutex autoplay_mutex;
   std::unique_lock<std::mutex> aplock{autoplay_mutex};
   {
      std::unique_lock<std::mutex> lock{m_mutex};

      StampObjProps();
      m_autoplay = x;
      if (m_autoplay)
      {
         if (m_timerThread)
         {
            m_timerThread->join();
            delete m_timerThread;
            m_timerThread = nullptr;
         }
         NextEvent();
         m_timerThread = new std::thread{[this] { autoplay_scheduler(); }};
      }
      else
      {
         m_CV.notify_all();
      }
   }
}

void FW2GUI::playdelay(float x)
{
   printf("playdelay %f\n", x);
   std::unique_lock<std::mutex> lock{m_mutex};
   m_deltaTime =  std::chrono::milliseconds(int(x));
   StampObjProps();
   m_CV.notify_all();
}

void
FW2GUI::RequestAddCollectionTable()
{
   using namespace  nlohmann;

   json top;
   top["c"] =  json::array();
   top["a"] =  json::array();

   std::vector<FWJobMetadataManager::Data> &usableData = m_main->getMetadataManager()->usableData();
   for ( auto i : usableData) {
      //if (i.purpose_ == "Table")
      //   continue;

      json j = json({});
      j["purpose"] = i.purpose_;
      j["moduleLabel"] = i.moduleLabel_;
      j["processName"] = i.processName_;
      j["productInstanceLabel"] = i.productInstanceLabel_;
      j["type"] = i.type_;

      if (i.isEDM)
         top["c"].push_back(j);
      else
         top["a"].push_back(j);
   }

   json jm;
   jm["arr"] = top;
   jm["action"] = "addCollectionResponse";
   std::string msg = "FW2_" + jm.dump();
   //td::cout << "ADD colleection " << msg.c_str();
   gEve->Send(0, msg.c_str());
}

void
FW2GUI::AddCollection(bool isEDM, const char* purpose, const char* moduleLabel,const char* productInstanceLabel,  const char* processName, const char* type)
{
   // std::cout << "AddCollection " << purpose << std::endl;
   FWDisplayProperties dp = FWDisplayProperties::defaultProperties;
   dp.setColor(kBlue);
   FWPhysicsObjectDesc desc("New-sth",  TClass::GetClass(type), purpose, dp, 
                           moduleLabel, productInstanceLabel, processName);
   m_main->addFW2Item(isEDM, desc);
}

void
FW2GUI::requestConfiguration()
{
   std::stringstream ss;

   FWConfiguration top;
   m_main->getConfigurationManager()->to(top);
   top.streamTo(ss, top, "top");

   nlohmann::json jm;
   jm["body"] = ss.str();
   jm["action"] = "saveConfigurationResponse";
   std::string msg = "FW2_" + jm.dump();
   gEve->Send(0, msg.c_str());

   std::ofstream file("tmp.fwc", std::ofstream::trunc);
   top.streamTo(file, top, "top");
}

void
FW2GUI::saveConfigurationAs(const char* path)
{
   std::string p = path;
   m_main->getConfigurationManager()->writeToFile(p);
   
}


int FW2GUI::WriteCoreJson(nlohmann::json &j, int rnr_offset)
{
   REveElement::WriteCoreJson(j, -1);
   const fwlite::Event* event = m_main->getCurrentEvent();

   j["fname"] = event->getTFile()->GetName();
   j["event"] = event->id().event();
   j["title"] = m_main->getFrameTitle();
   j["run"] = event->id().run();
   j["lumi"] = event->id().luminosityBlock();
   j["date"] = fireworks::getLocalTime( *event ).c_str();
   j["size"] = event->size();
   j["standalone"] =  m_main->isStandalone();
   j["UT_PostStream"] = "UT_refresh_event_info";

   j["autoplay"] = m_autoplay;

   std::chrono::milliseconds ms(1);
   std::chrono::seconds sec(1);
   int msc = 0;// m_deltaTime(sec).count();
    msc = std::chrono::duration_cast<std::chrono::minutes>(m_deltaTime).count();
   j["playdelay"] = msc;

   return 0;
}
