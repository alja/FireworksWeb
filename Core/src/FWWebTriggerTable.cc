#include <iostream>
#include "FireworksWeb/Core/interface/FWWebTriggerTable.h"
#include "FireworksWeb/Core/interface/fwLog.h"
#include "FireworksWeb/Core/interface/Context.h"

#include "DataFormats/FWLite/interface/Handle.h"
#include "DataFormats/FWLite/interface/Event.h"
#include "FWCore/Common/interface/TriggerNames.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "FWCore/Utilities/interface/Exception.h"

#include "nlohmann/json.hpp"


//------------------------------------------------------------------------

FWWebTriggerTable::FWWebTriggerTable(const fwlite::Event* event)
{
    m_event = (fwlite::Event*)event;
}

//------------------------------------------------------------------------
void FWWebTriggerTable::readTriggerData() {

    ///!!!! Check for new event !!! amr
  StampObjProps();
}

//------------------------------------------------------------------------
void FWWebTriggerTable::fillAverageAcceptFractions() {
  edm::EventID currentEvent = m_event->id();
  // better to keep the keys and just set to zero the values
  for (acceptmap_t::iterator it = m_averageAccept.begin(), ed = m_averageAccept.end(); it != ed; ++it) {
    it->second = 0;
  }

  // loop over events
  fwlite::Handle<edm::TriggerResults> hTriggerResults;
  for (m_event->toBegin(); !m_event->atEnd(); ++(*m_event)) {
    hTriggerResults.getByLabel(*m_event, "TriggerResults", "", "HLT");
    edm::TriggerNames const* triggerNames(nullptr);
    try {
      triggerNames = &m_event->triggerNames(*hTriggerResults);
    } catch (cms::Exception&) {
      fwLog(fwlog::kInfo) << "  FWWebTriggerTable::fillAverageAcceptFractionsexception missing trigger info" << std::endl;
      break;
    }

    for (unsigned int i = 0; i < triggerNames->size(); ++i) {
      if (hTriggerResults->accept(i)) {
        m_averageAccept[triggerNames->triggerName(i)]++;
      }
    }
  }
  m_event->to(currentEvent);

  double denominator = 1.0 / m_event->size();
  for (acceptmap_t::iterator it = m_averageAccept.begin(), ed = m_averageAccept.end(); it != ed; ++it) {
    it->second *= denominator;
  }
}

//------------------------------------------------------------------------

int FWWebTriggerTable::WriteCoreJson(nlohmann::json &j, int rnr_offset)
{
    int ret = REveElement::WriteCoreJson(j, rnr_offset);
    j["name"] = nlohmann::json::array();
    j["result"] = nlohmann::json::array();
    j["average"] = nlohmann::json::array();

    try
    {
        m_event = (fwlite::Event *)fireworks::Context::getInstance()->getCurrentEvent();
        fillAverageAcceptFractions();
        fwlite::Handle<edm::TriggerResults> hTriggerResults;
        edm::TriggerNames const *triggerNames(nullptr);
        hTriggerResults.getByLabel(*m_event, "TriggerResults", "", "HLT");
        triggerNames = &m_event->triggerNames(*hTriggerResults);
        for (unsigned int i = 0; i < triggerNames->size(); ++i)
        {
          j["name"].push_back(triggerNames->triggerName(i));
          j["result"].push_back(hTriggerResults->accept(i) ? "1" : "0");
          j["average"].push_back(Form("%6.1f", m_averageAccept[triggerNames->triggerName(i)] * 100));
        }
    }
    catch (cms::Exception &)
    {
        std::cerr << "EXIT  no trigger results with process name HLT is available" << std::endl;
    }
    return ret;
}
