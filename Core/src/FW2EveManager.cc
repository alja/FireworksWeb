
#include "FireworksWeb/Core/interface/FW2EveManager.h"

#include "TRandom.h"
#include <ROOT/REveManager.hxx>
#include <ROOT/REveTableProxyBuilder.hxx>
#include <ROOT/REveTableInfo.hxx>
#include <ROOT/REveCalo.hxx>
#include <ROOT/REveDataProxyBuilderBase.hxx>
#include <ROOT/REveScene.hxx>
#include <ROOT/REveSelection.hxx>
#include <ROOT/REveViewer.hxx>
#include "ROOT/REveViewContext.hxx"


#include "FireworksWeb/Core/interface/FWSimpleRepresentationChecker.h"
#include "FireworksWeb/Core/interface/FWEDProductRepresentationChecker.h"
#include "FireworksWeb/Core/interface/FWProxyBuilderFactory.h"
#include "FWCore/PluginManager/interface/PluginFactory.h"
#include "FWCore/Utilities/interface/Exception.h"

#include "FireworksWeb/Core/interface/FWEveView.h"
#include "FireworksWeb/Core/interface/FWRPZView.h"
#include "FireworksWeb/Core/interface/FWTableViewManager.h"
#include "FireworksWeb/Core/interface/Context.h"
#include "FireworksWeb/Core/interface/fwLog.h"
#include "FireworksWeb/Core/interface/FWEventItem.h"
#include "FireworksWeb/Core/interface/FWGeometry.h"
#include "FireworksWeb/Core/interface/FWViewEnergyScale.h"


using namespace ROOT::Experimental;

FW2EveManager::FW2EveManager(FWTableViewManager* iTableMng):
   m_tableManager(iTableMng)
{
   m_viewContext = new REveViewContext();
   m_viewContext->SetTableViewInfo(m_tableManager->getTableInfo());
   //  initTypeToBuilder();
   //createScenesAndViews();
}
//______________________________________________________________________________
void FW2EveManager::initTypeToBuilder()
{
    std::set<std::string> builders;
    // AMT why check for polugins intwo different ways
    {
       std::vector<edmplugin::PluginInfo> available = FWProxyBuilderFactory::get()->available();
        for(auto& it : available) {
          builders.insert(it.name_);
       }
    }
    {
       std::vector<edmplugin::PluginInfo> available = edmplugin::PluginManager::get()->categoryToInfos().find(FWProxyBuilderFactory::get()->category())->second;
       for(auto& it : available) {
          builders.insert(it.name_);
       }
    }   
   
   for(std::set<std::string>::iterator it = builders.begin(), itEnd=builders.end();
       it!=itEnd;
       ++it) {
      std::string::size_type first = it->find_first_of('@')+1;
      std::string purpose = it->substr(first,it->find_last_of('@')-first);
      
      //first = it->find_last_of('@')+1;
      // std::string view_str =  it->substr(first,it->find_last_of('#')-first);
      // int viewTypes = atoi(view_str.c_str());
      std::string fullName = *it;
      // printf("register builde purpose: %s fullName %s \n", purpose.c_str(), fullName.c_str());
      m_typeToBuilder[purpose].push_back(BuilderInfo(fullName));
   }
}
//______________________________________________________________________________
void FW2EveManager::createScenesAndViews()
{
   // disable default view
   gEve->GetViewers()->FirstChild()->SetRnrSelf(false);

  if (1) {
      auto view = new FW3DView("3D");
      m_views.push_back(view);
      view->importContext(m_viewContext);
   }

   {
      auto view = new FWRPZView("RPhi");
      m_views.push_back(view);
      view->importContext(m_viewContext);
   }
   {
      auto view = new FWRPZView("RhoZ");
      m_views.push_back(view);
      view->importContext(m_viewContext);
   }
   
   {
      auto view = new FWLegoView("Lego");
      m_views.push_back(view);
      view->importContext(m_viewContext);
      view->viewer()->SetRnrSelf(false);
   }
   {
      auto view = new FWTableView("Table");
      m_views.push_back(view);
      view->importContext(m_viewContext);
   }
   {
      auto view = new FWTriggerTableView("TriggerTable");
      m_views.push_back(view);
      view->importContext(m_viewContext);
      view->viewer()->SetRnrSelf(false);
   }
}

//______________________________________________________________________________
void FW2EveManager::newItem(FWEventItem *iItem)
{
   try
   {
      auto collection = iItem;//->getCollection();
      TypeToBuilder::iterator itFind = m_typeToBuilder.find(iItem->purpose());

      if (itFind == m_typeToBuilder.end())
         return;

      std::vector<BuilderInfo> &blist = itFind->second;

      std::string bType;
      bool bIsSimple;
      for (size_t bii = 0, bie = blist.size(); bii != bie; ++bii)
      {
         // 1.
         BuilderInfo &info = blist[bii];
         info.classType(bType, bIsSimple);

         if (bIsSimple)
         {
            unsigned int distance = 1;
            edm::TypeWithDict modelType(*(iItem->modelType()->GetTypeInfo()));
            if (!FWSimpleRepresentationChecker::inheritsFrom(modelType, bType, distance))
            {
               // printf("PB does not matche itemType (%s) !!! EDproduct %s %s\n", info.m_name.c_str(), iItem->modelType()->GetTypeInfo()->name(), bType.c_str() );
               continue;
            }
         }
         else
         {
            std::string itype = iItem->type()->GetTypeInfo()->name();
            if (itype != bType)
            {
               // printf("PB does not match modeType (%s)!!! EDproduct %s %s\n", info.m_name.c_str(), itype.c_str(), bType.c_str() );
               continue;
            }
         }

         std::string builderName = info.m_name;
         std::shared_ptr<FWProxyBuilderBase> builder;

         try
         {
            auto builder = FWProxyBuilderFactory::get()->create(builderName);
            addGraphicalProxyBuilder(collection, builder.release());
         }
         catch (std::exception &exc)
         {
            fwLog(fwlog::kWarning)
                << "FWEveViewManager::newItem ignoring the following exception (probably edmplugincache mismatch):"
                << std::endl
                << exc.what();
         }
         if (!builder)
            continue;
      }

      // don't need a plugin for table view
      addTableProxyBuilder(collection);

      // tooltips
      auto tableInfo = m_viewContext->GetTableViewInfo();
      auto tableEntries = tableInfo->RefTableEntries(collection->GetItemClass()->GetName());
      int N = TMath::Min(int(tableEntries.size()), 3);
      for (int t = 0; t < N; t++)
      {
         auto te = tableEntries[t];
         collection->GetItemList()->AddTooltipExpression(te.fName, te.fExpression, false);
      }

      // connect to signals
      collection->GetItemList()->SetFillImpliedSelectedDelegate([&](REveDataItemList *collection, REveElement::Set_t &impSelSet, const std::set<int> &sec_idcs)
                                                                { this->FillImpliedSelected(collection, impSelSet, sec_idcs); });
      collection->GetItemList()->SetItemsChangeDelegate([&](REveDataItemList *collection, const REveDataCollection::Ids_t &ids)
                                                        { this->modelChanged(collection, ids); });
   }
   catch (const cms::Exception &iE)
   {
      std::cout << iE << std::endl;
   }
}

//______________________________________________________________________________
void FW2EveManager::addTableProxyBuilder(REveDataCollection *collection)
{
   static std::string dc = "testEmpty";

   auto tableInfo = m_viewContext->GetTableViewInfo();
   auto tableBuilder = new REveTableProxyBuilder();
   tableBuilder->SetHaveAWindow(true);
   tableBuilder->SetCollection(collection);
   REveElement *tablep = tableBuilder->CreateProduct("table-type", m_viewContext);

   tableInfo->AddDelegate([=]() { tableBuilder->ConfigChanged(); });

   bool buildTable = false;
   if (dc.empty() || (m_tableManager->getDisplayedCollection().compare(collection->GetName()) == 0))
   {
      dc = collection->GetName();
      tableInfo->SetDisplayedCollection(collection->GetElementId());
      buildTable = true;
   }

   for (auto &ev : m_views)
   {
      if (ev->viewType() == "Table")
      {
         ev->eventScene()->AddElement(tablep);
         if (buildTable)
         {
            m_tableManager->checkExpressionsForType(collection->GetItemClass());
            tableBuilder->Build();
         }
      }
   }

   m_builders.push_back(tableBuilder);
}

//______________________________________________________________________________
void FW2EveManager::addGraphicalProxyBuilder(REveDataCollection *collection, REveDataProxyBuilderBase *builder)
{
   builder->SetCollection(collection);
   builder->SetHaveAWindow(true);

   static float depth = 1.0f;

   REveElement *singleProduct = nullptr;
   if (builder->HaveSingleProduct())
       singleProduct = builder->CreateProduct("3D", m_viewContext);

   for (auto &ev : m_views)
   {
      // disable default view
      if (!ev->viewer()->GetRnrSelf())
        continue;

      if (ev->viewType() == "Table" ||ev->viewType() == "TriggerTable" || ev->viewType() == "Lego")
      {
         continue;
      }

      if (ev->viewType() == "3D")
      {
         if (builder->HaveSingleProduct())
         {
            ev->eventScene()->AddElement(singleProduct);
         }
         else
         {
            auto perViewProduct = builder->CreateProduct(ev->viewType(), m_viewContext);
            ev->eventScene()->AddElement(perViewProduct);
         }
      }
      else
      {
         FWRPZView *rpzv = dynamic_cast<FWRPZView *>(ev);
         if (builder->HaveSingleProduct())
         {
            rpzv->importElements(singleProduct, depth, rpzv->eventScene());
         }
         else
         {
            auto perViewProd = builder->CreateProduct(ev->viewType(), m_viewContext);
            rpzv->importElements(perViewProd, depth, rpzv->eventScene());
         }
      }
   }
   m_builders.push_back(builder);
   builder->Build();
}

//______________________________________________________________________________
void FW2EveManager::beginEvent()
{
   m_acceptChanges=false;
   fireworks::Context::getInstance()->resetMaxEtAndEnergy();

   gEve->GetSelection()->ClearSelection();
   gEve->GetHighlight()->ClearSelection();

   for (auto &ev : m_views)
      ev->eventBegin();
}

//______________________________________________________________________________
void FW2EveManager::endEvent()
{
   for ( auto &i : m_builders) {
      i->Build();
   }

   for (auto &ev : m_views)
      ev->eventEnd();

   for (auto proxy : m_builders)
      proxy->ScaleChanged();

   m_acceptChanges = true;
}

//______________________________________________________________________________
void FW2EveManager::globalEnergyScaleChanged()
{
   //ROOT::Experimental::REveManager::ChangeGuard ch;
   printf("Eve Manger SCALE changed\n");
   for (auto &ev : m_views)
      ev->setupEnergyScale();

   for (auto proxy : m_builders)
      proxy->ScaleChanged();

   fireworks::Context::getInstance()->energyScale()->StampObjProps();
}

//______________________________________________________________________________
void FW2EveManager::modelChanged(REveDataItemList* itemList, const REveDataCollection::Ids_t& ids) {
   if (!m_acceptChanges)
      return;
   
   for (auto proxy : m_builders) {
      if (proxy->Collection()->GetItemList() == itemList) {
         // printf("Model changes check proxy %s: %lu \n", proxy->Collection()->GetCName(), ids.size());
         proxy->ModelChanges(ids);
      }
   }
}
//______________________________________________________________________________
void FW2EveManager::FillImpliedSelected(REveDataItemList *itemList, REveElement::Set_t &impSelSet, const std::set<int>& sec_idcs)
{
   if (!m_acceptChanges)
      return;

   for (auto &proxy : m_builders)
   {
      if (proxy->Collection()->GetItemList() == itemList)
      {
         proxy->FillImpliedSelected(impSelSet, sec_idcs);
      }
   }
}

//______________________________________________________________________________
void FW2EveManager::BuilderInfo::classType(std::string &typeName, bool &simple) const
{
   const std::string kSimple("simple#");
   simple = (m_name.substr(0, kSimple.size()) == kSimple);
   if (simple)
   {
      typeName = m_name.substr(kSimple.size(), m_name.find_first_of('@') - kSimple.size());
   }
   else
   {
      typeName = m_name.substr(0, m_name.find_first_of('@'));
   }
}
//______________________________________________________________________________
FWTypeToRepresentations
FW2EveManager::supportedTypesAndRepresentations() const
{
   FWTypeToRepresentations returnValue;
   for (TypeToBuilder::const_iterator it = m_typeToBuilder.begin(), itEnd = m_typeToBuilder.end();
        it != itEnd;
        ++it)
   {
      std::vector<BuilderInfo> blist = it->second;
      for (size_t bii = 0, bie = blist.size(); bii != bie; ++bii)
      {
         BuilderInfo &info = blist[bii];
         std::string name;
         bool representsSubPart = false;
         unsigned int bitPackedViews = 0;
         bool FFOnly = false;
         bool isSimple = true;
         info.classType(name, isSimple);
         // std::cout << "supportedTypesAndRepresentations class type " << name << "!!!!!! \n";

         if (isSimple)
         {
            returnValue.add(std::make_shared<FWSimpleRepresentationChecker>(name, it->first, bitPackedViews, representsSubPart, FFOnly));
         }
         else
         {
            representsSubPart = true;
            returnValue.add(std::make_shared<FWEDProductRepresentationChecker>(name, it->first, bitPackedViews, representsSubPart, FFOnly));
         }
      }
   }

   return returnValue;
}
