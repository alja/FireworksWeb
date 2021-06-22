
#include "FireworksWeb/Core/interface/FWGUIEventFilter.h"
#include "FireworksWeb/Core/interface/CmsShowNavigator.h"
#include "FireworksWeb/Core/interface/FWEventSelector.h"

#include "TBase64.h"

using namespace ROOT::Experimental;

FWGUIEventFilter::FWGUIEventFilter(CmsShowNavigator *n) : REveElement("GUIEventFilter"),
                                                          m_navigator(n)
{
}

FWGUIEventFilter::~FWGUIEventFilter() {
}

void FWGUIEventFilter::SetFilterEnabled(bool /*on*/)
{
   m_navigator->toggleFilterEnable();
}

void FWGUIEventFilter::PublishFilters(const char *arg)
{
   // automatically switch on --double  check with original ???
   if (m_navigator->m_filterState == CmsShowNavigator::kOff)
   {
      m_navigator->m_filesNeedUpdate = true;
      m_navigator->m_filterState = CmsShowNavigator::kOn;
   }

   using namespace nlohmann;
   TString test = TBase64::Decode(arg);
   std::string msg = test.Data();
   json j = json::parse(msg);
   std::cout << j.dump(4);
   std::list<FWEventSelector> guiSelectors;
   for (json::iterator it = j["modelData"].begin(); it != j["modelData"].end(); ++it)
   {
      json fo = *it;
      FWEventSelector p(fo["id"]);
      p.m_expression = fo["expr"];
      p.m_enabled = fo["enabled"];
      guiSelectors.push_back(p);
   }
   for (json::iterator it = j["hltData"].begin(); it != j["hltData"].end(); ++it)
   {
      json fo = *it;
      FWEventSelector p(fo["id"]);
      p.m_expression = fo["expr"];
      p.m_enabled = fo["enabled"];
      p.m_triggerProcess = fo["trigger"];
      guiSelectors.push_back(p);
   }

   std::list<FWEventSelector *>::iterator si = m_navigator->m_selectors.begin();
   std::list<FWEventSelector>::iterator gi = guiSelectors.begin();

   while (si != m_navigator->m_selectors.end() || gi != guiSelectors.end())
   {
      if (gi == guiSelectors.end() && si != m_navigator->m_selectors.end())
      {
         //printf("remove filter ....%s\n", (*si)->m_expression.c_str());
         m_navigator->removeFilter(si++);
      }
      else if (si == m_navigator->m_selectors.end() && gi != guiSelectors.end())
      {
         //printf("add filter %s...\n", gi->m_expression.c_str());
         m_navigator->addFilter(*gi);
         ++gi;
      }
      else
      {
         //printf("compare id %d %d\n", (*si)->m_id, gi->m_id);
         if ((*si)->m_id == (*gi).m_id)
         {
            bool filterNeedUpdate = gi->m_expression != (*si)->m_expression;
            if (filterNeedUpdate || gi->m_enabled != (*si)->m_enabled)
            {
               (*si)->m_expression = gi->m_expression;
               (*si)->m_enabled = gi->m_enabled;
               m_navigator->changeFilter(*si, filterNeedUpdate);
            }
            ++si;
            ++gi;
         }
         else if ((*gi).m_id == -1)
         {
            //printf("add filter %s...\n", (*gi).m_expression.c_str());
            m_navigator->addFilter(*gi);
            ++gi;
         }
         else
         {
            //printf("remove filter %s...\n", (*si)->m_expression.c_str());
            m_navigator->removeFilter(si++);
         }
      }
   }

   if (m_navigator->m_filesNeedUpdate)
      m_navigator->updateFileFilters();

   StampObjProps();
}
//----------------------------------------------------------

int FWGUIEventFilter::WriteCoreJson(nlohmann::json &j, int rnr_offset)
{
   REveElement::WriteCoreJson(j, -1);
   // printf("FWGUIEventFilter::WriteCoreJson ENABLED %d\n", m_navigator->m_filterState);

   j["UT_PostStream"] = "UT_refresh_filter_info";

   j["enabled"] = m_navigator->m_filterState == 1 ? 1 : 0;
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
  // std::cout << "json " << j.dump(5) << std::endl;
   return 0;
}
