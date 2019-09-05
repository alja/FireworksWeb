#include "Fireworks2/Core/interface/FW2GUI.h"
#include "Fireworks2/Core/interface/BuilderUtils.h"
#include "Fireworks2/Core/src/json.hpp"

#include "DataFormats/FWLite/interface/Event.h"

#include "TFile.h"

using namespace ROOT::Experimental;

FW2GUI::FW2GUI() : m_main(0) {
}

FW2GUI::FW2GUI(FW2Main* m) : m_main(m), m_ecnt(1) {
}

FW2GUI::~FW2GUI()
{
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
   return 0;
}
