
#include "FireworksWeb/Core/interface/FWGUIEventFilter.h"
#include "FireworksWeb/Core/interface/CmsShowNavigator.h"
#include "FireworksWeb/Core/interface/FWEventSelector.h"
using namespace ROOT::Experimental;

FWGUIEventFilter::FWGUIEventFilter(CmsShowNavigator* n):
REveElement("GUIEventFilter"),
m_navigator(n)
{
    
}

FWGUIEventFilter::~FWGUIEventFilter() {}

int FWGUIEventFilter::WriteCoreJson(nlohmann::json &j, int rnr_offset)
{
   int res = REveElement::WriteCoreJson(j, rnr_offset);

   j["enabled"] = m_navigator->m_filterState;
   j["collection"] = nlohmann::json::array();
   j["HLT"] = nlohmann::json::array();
   for (auto &s : m_navigator->m_selectors)
   {
      std::cout << s->m_expression;

      nlohmann::json f{
          {"expr", s->m_expression},
          {"enabled", s->m_enabled},
          {"trigger", s->m_triggerProcess},
          {"selected", s->m_selected}};

      if (s->m_triggerProcess.empty())
      {
         j["collection"].push_back(f);
      }
      else
      {
         j["HLT"].push_back(f);
      }
   }
   return res;
}
