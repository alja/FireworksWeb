#include "Fireworks2/Core/interface/FW2GUI.h"
#include "Fireworks2/Core/interface/BuilderUtils.h"
//#include "Fireworks2/Core/src/json.hpp"
#include "Fireworks2/Core/interface/FWJobMetadataManager.h"
#include "Fireworks2/Core/interface/FWLiteJobMetadataManager.h"
#include "Fireworks2/Core/interface/FWDisplayProperties.h"
#include "Fireworks2/Core/interface/FWPhysicsObjectDesc.h"
#include "DataFormats/FWLite/interface/Event.h"

#include "TFile.h"

using namespace ROOT::Experimental;

FW2GUI::FW2GUI() : m_main(0) {
   m_deltaTime = std::chrono::milliseconds(500);
}

FW2GUI::FW2GUI(FW2Main* m) : m_main(m), m_ecnt(1) {
}

FW2GUI::~FW2GUI()
{
}


void FW2GUI::terminate()
{
   exit(0);
}

void
FW2GUI::NextEvent()
{
   printf("next event \n");
   m_main->nextEvent();
}

void
FW2GUI::PreviousEvent()
{
   m_main->previousEvent();
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
   printf("auto play \n");
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
FW2GUI::AddCollection(const std::string& purpose, const std::string& label, const std::string& process, const std::string& type)
{
   // std::cout << "AddCollection " << purpose << std::endl;
   FWDisplayProperties dp = FWDisplayProperties::defaultProperties;
   dp.setColor(kBlue);
   FWPhysicsObjectDesc desc("New-sth",  TClass::GetClass(type.c_str()), purpose.c_str(), dp, label.c_str());
   m_main->addFW2Item(desc);
   gEve->Redraw3D();
}


int FW2GUI::WriteCoreJson(nlohmann::json &j, int rnr_offset)
{
   REveElement::WriteCoreJson(j, -1);
   const fwlite::Event* event = m_main->getCurrentEvent();

   j["fname"] = event->getTFile()->GetName();
   j["event"] = event->id().event();
   j["eventCnt"] = m_ecnt;
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
