
#include <fstream>

#include "FireworksWeb/Core/interface/FW2GUI.h"
#include "FireworksWeb/Core/interface/BuilderUtils.h"
//#include "FireworksWeb/Core/src/json.hpp"
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

   json top =  json::array();

   std::vector<FWJobMetadataManager::Data> &usableData = m_main->getMetadataManager()->usableData();
   for ( auto i : usableData) {
      if (i.purpose_ == "Table")
         continue;

      json j = json({});
      j["purpose"] = i.purpose_;
      j["moduleLabel"] = i.moduleLabel_;
      j["processName"] = i.processName_;
      j["type"] = i.type_;
      top.push_back(j);
   }

   json jm;
   jm["arr"] = top;
   jm["action"] = "addCollectionResponse";
   std::string msg = "FW2_" + jm.dump();
   //td::cout << "ADD colleection " << msg.c_str();
   gEve->Send(0, msg.c_str());
}

void
FW2GUI::requestConfiguration()
{
   printf("request configuration \n");
   std::stringstream ss;

    FWConfiguration top;
    m_main->getConfigurationManager()->to(top);
    top.streamTo(ss, top, "top");
   //FWConfiguration::streamTo(ss, *m_main->getConfigurationManager(), "top");
  ///std::cout << "-----\n" << ss.str();


   std::ofstream file("tmp.fwc", std::ofstream::trunc);
   top.streamTo(file, top, "top");
}


void
FW2GUI::AddCollection(const char* purpose, const char* label, const char* process, const char* type)
{
   // std::cout << "AddCollection " << purpose << std::endl;
   FWDisplayProperties dp = FWDisplayProperties::defaultProperties;
   dp.setColor(kBlue);
   FWPhysicsObjectDesc desc("New-sth",  TClass::GetClass(type), purpose, dp, label);
   m_main->addFW2Item(desc);
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
   j["UT_PostStream"] = "UT_refresh_event_info";

   j["autoplay"] = m_autoplay;

   std::chrono::milliseconds ms(1);
   std::chrono::seconds sec(1);
   int msc = 0;// m_deltaTime(sec).count();
    msc = std::chrono::duration_cast<std::chrono::minutes>(m_deltaTime).count();
   j["playdelay"] = msc;

   return 0;
}
