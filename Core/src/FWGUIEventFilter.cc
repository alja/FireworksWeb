
#include "FireworksWeb/Core/interface/FWGUIEventFilter.h"
#include "FireworksWeb/Core/interface/CmsShowNavigator.h"
#include "FireworksWeb/Core/interface/FWEventSelector.h"

#include "TBase64.h"

using namespace ROOT::Experimental;

FWGUIEventFilter::FWGUIEventFilter(CmsShowNavigator *n) : REveElement("GUIEventFilter"),
                                                          m_navigator(n)
{
}

FWGUIEventFilter::~FWGUIEventFilter() {}

void FWGUIEventFilter::SetFilterEnabled(bool on)
{
   on ? m_navigator->resumeFilter() : m_navigator->withdrawFilter();
}

void FWGUIEventFilter::PublishFilters(const char *arg)
{
   using namespace nlohmann;
   TString test = TBase64::Decode(arg);
   std::string msg = test.Data();
   json j = json::parse(msg);
   std::vector<FWEventSelector> flist;

   for (json::iterator it = j["modelData"].begin(); it != j["modelData"].end(); ++it)
   {
      FWEventSelector p;
      json fo = *it;
      p.m_expression = fo["expr"];
      p.m_enabled = fo["enabled"];
      flist.push_back(p);
   }
   for (json::iterator it = j["hltData"].begin(); it != j["hltData"].end(); ++it)
   {
      json fo = *it;
      FWEventSelector p;
      p.m_expression = fo["expr"];
      p.m_enabled = fo["enabled"];
      p.m_triggerProcess = fo["trigger"];
      flist.push_back(p);
   }
   /*
   printf("fffffffffffffffffffffffffffffffffffffffffffffffffffff\n");
   using namespace nlohmann;
   TString test = TBase64::Decode(arg);
   std::cout << "data " << test.Data() << std::endl;
   std::string msg = test.Data();

   std::cout << "dump ....\n";
   json j = json::parse(msg);
   std::string xx = j.dump(5);
   std::cout << xx << std::endl;

   std::vector<FWEventSelector*> flist;

   printf("ffffffffffffffffff22222222222222222222222\n");
   for (json::iterator it = j["modelData"].begin(); it != j["modelData"].end(); ++it)
   {
     // std::shared_ptr<FWEventSelector> p = std::make_shared<FWEventSelector>();
      FWEventSelector* p = new FWEventSelector();
      json fo = *it;
      p->m_expression = fo["expr"];
      p->m_enabled = fo["enabled"];
      flist.push_back(p);
   }
   for (json::iterator it = j["hltData"].begin(); it != j["hltData"].end(); ++it)
   {
      json fo = *it;
     // std::shared_ptr<FWEventSelector> p = std::make_shared<FWEventSelector>();
      //std::shared_ptr<FWEventSelector> p = std::make_shared<FWEventSelector>();
      FWEventSelector* p = new FWEventSelector();
      p->m_expression = fo["expr"];
      p->m_enabled = fo["enabled"];
      p->m_triggerProcess = fo["trigger"];
    flist.push_back(p);
   }*/

   m_navigator->applyFiltersFromGUI(arg);
}

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
