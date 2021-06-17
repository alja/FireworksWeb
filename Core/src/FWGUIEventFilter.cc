
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
      p.m_id = fo["id"];
      flist.push_back(p);
   }
   for (json::iterator it = j["hltData"].begin(); it != j["hltData"].end(); ++it)
   {
      json fo = *it;
      FWEventSelector p;
      p.m_expression = fo["expr"];
      p.m_enabled = fo["enabled"];
      p.m_triggerProcess = fo["trigger"];
      p.m_id = fo["id"];
      flist.push_back(p);
   }

   for (auto &ns : m_navigator->m_selectors)
   {
      //  bool found = false;
      for (auto &gsr : flist)
      {
         FWEventSelector *gs = &gsr;
         if (gs->m_id == ns->m_id)
         {
            bool filterNeedUpdate = gs->m_expression != ns->m_expression;
            if (filterNeedUpdate || gs->m_enabled != ns->m_enabled)
            {
               ns->m_expression = gs->m_expression;
               ns->m_enabled = gs->m_enabled;
               m_navigator->changeFilter(ns, filterNeedUpdate);
            }
         }
      }

      if (m_navigator->m_filesNeedUpdate)
         m_navigator->updateFileFilters();

   }

   m_navigator->applyFiltersFromGUI(arg);
   StampObjProps();   
}

int FWGUIEventFilter::WriteCoreJson(nlohmann::json &j, int rnr_offset)
{
   REveElement::WriteCoreJson(j, -1);

   j["UT_PostStream"] = "UT_refresh_filter_info";

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
          {"selected", s->m_selected},
          {"id", s->m_id}};

      if (s->m_triggerProcess.empty())
      {
         j["collection"].push_back(f);
      }
      else
      {
         j["HLT"].push_back(f);
      }
   }
   std::cout << "json " << j.dump(5) << std::endl;
   return 0;
}
