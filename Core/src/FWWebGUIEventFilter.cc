
#include "FireworksWeb/Core/interface/FWWebGUIEventFilter.h"
#include "FireworksWeb/Core/interface/CmsShowNavigator.h"
#include "FireworksWeb/Core/interface/FWEventSelector.h"

#include "TBase64.h"
#include "nlohmann/json.hpp"

using namespace ROOT::Experimental;

FWWebGUIEventFilter::FWWebGUIEventFilter(CmsShowNavigator *n) : REveElement("GUIEventFilter"),
                                                          m_navigator(n)
{
}

FWWebGUIEventFilter::~FWWebGUIEventFilter() {
}

void FWWebGUIEventFilter::toggleFilterEnabled()
{
   m_navigator->toggleFilterEnable();
   StampObjProps();
}

void FWWebGUIEventFilter::setFilterEnabled(bool x)
{
   if (x) {
      m_navigator->m_filesNeedUpdate = true;
      m_navigator->m_filterState = CmsShowNavigator::kOn;
   }
   else {
      m_navigator->m_filterState = CmsShowNavigator::kOff;
   }
   StampObjProps();
}

void FWWebGUIEventFilter::PublishFilters(const char *arg)
{
   // automatically switch on --double  check with original ???
   if (m_navigator->m_filterState == CmsShowNavigator::kOff)
   {
      m_navigator->m_filesNeedUpdate = true;
      m_navigator->m_filterState = CmsShowNavigator::kOn;
   }

   using namespace nlohmann;
   std::string msg( TBase64::Decode(arg).Data() );
   json j = json::parse(msg);
   // std::cout << "\n==== Filter state: " << j.dump(4) << std::endl;
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

   int nm = 0;
   try
   {
      nm = j["filterMode"].get<int>();
   }
   catch (std::exception &e)
   {
      std::cout << "FWWebGUIEventFilter::PublishFilters " << e.what() << std::endl;
   }
   
   
   if (nm != m_navigator->m_filterMode)
   {
      m_navigator->m_filterMode = nm;
      m_navigator->m_filesNeedUpdate = true;
   }

   if (m_navigator->m_filesNeedUpdate)
      m_navigator->updateFileFilters();

   StampObjProps();
}
//----------------------------------------------------------

int FWWebGUIEventFilter::WriteCoreJson(nlohmann::json &j, int rnr_offset)
{
   REveElement::WriteCoreJson(j, -1);
   printf(">>>> FWWebGUIEventFilter::WriteCoreJson filterState ...statusID %d\n", m_navigator->m_filterState);

   j["UT_PostStream"] = "UT_refresh_filter_info";

   j["statusID"] = m_navigator->m_filterState;
   j["filterMode"] = m_navigator->m_filterMode;
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

      j["NSelected"] = m_navigator->getNSelectedEvents();
      j["NTotal"] = m_navigator->getNTotalEvents();
   }
   // std::cout << "json " << j.dump(5) << std::endl;
   return 0;
}
