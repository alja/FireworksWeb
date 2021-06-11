
#include "FireworksWeb/Core/interface/FWGUIEventFilter.h"
#include "FireworksWeb/Core/interface/CmsShowNavigator.h"
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

   j["filterState"] = m_navigator->m_filterState;
   return res;
}
