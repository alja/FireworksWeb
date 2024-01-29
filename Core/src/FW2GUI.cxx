
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

void FW2GUI::setAutoplay(bool x)
{
   fwLog(fwlog::kInfo) << "Set autoplay " << x << std::endl;
   StampObjProps();
   m_main->do_set_autoplay(x);
}

// set play delay in miliseconds
void FW2GUI::setPlayDelayInMiliseconds(float x)
{
   StampObjProps();
   fwLog(fwlog::kInfo) << "FW2GUI::playdelay " << x << std::endl;
   m_playdelay = x;
   m_main->do_set_playdelay(x);
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
      j["bad"] = i.empty_;


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
   if (event) {
      j["fname"] = event->getTFile()->GetName();
      j["event"] = event->id().event();
      j["title"] = m_main->getFrameTitle();
      j["run"] = event->id().run();
      j["lumi"] = event->id().luminosityBlock();
      j["date"] = fireworks::getLocalTime( *event ).c_str();
      j["size"] = event->size();
   }
   else {
      j["event"] = 99;
      j["title"] = "undefined [0/0]";
      j["run"] = 99;
      j["lumi"] = 99;
      j["date"] = 0;
      j["size"] = 0;
   }

   j["standalone"] =  m_main->isStandalone();
   j["UT_PostStream"] = "UT_refresh_event_info";
   j["autoplay"] = m_main->isPlaying();
   j["nav"] = nlohmann::json::array();
   m_main->setGUICtrlStates();
   for(auto &s : m_ctrlStates) {
      j["nav"].push_back(s);
      // std::cout << "GUI nav " << s << "\n";
   }
   j["playdelay"] = m_playdelay;

   // std::cout << "FW2GUI::WriteCoreJson " << j.dump(3) << "\n";

   return 0;
}
