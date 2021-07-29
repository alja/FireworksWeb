
#include "FireworksWeb/Core/interface/FW2EveManager.h"

#include <ROOT/REveManager.hxx>
#include <ROOT/REveTableProxyBuilder.hxx>
#include <ROOT/REveTableInfo.hxx>
#include <ROOT/REveCalo.hxx>
#include <ROOT/REveDataProxyBuilderBase.hxx>
#include <ROOT/REveScene.hxx>
#include <ROOT/REveViewer.hxx>
#include "ROOT/REveViewContext.hxx"


#include "FireworksWeb/Core/interface/FWSimpleRepresentationChecker.h"
#include "FireworksWeb/Core/interface/FWProxyBuilderFactory.h"
#include "FWCore/PluginManager/interface/PluginFactory.h"
#include "FWCore/Utilities/interface/Exception.h"

#include "FireworksWeb/Core/interface/FWEveView.h"
#include "FireworksWeb/Core/interface/FWRPZView.h"
#include "FireworksWeb/Core/interface/FWTableViewManager.h"
#include "FireworksWeb/Core/interface/Context.h"
#include "FireworksWeb/Core/interface/FWEventItem.h"
#include "FireworksWeb/Core/interface/FWGeometry.h"


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
   }
   {
      auto view = new FWTableView("Table");
      m_views.push_back(view);
      view->importContext(m_viewContext);
   }
}

//______________________________________________________________________________
void FW2EveManager::newItem(FWEventItem* iItem)
{
   try
   {
      auto collection = iItem->getCollection();
      if (edmplugin::PluginManager::get()->categoryToInfos().end() != edmplugin::PluginManager::get()->categoryToInfos().find(FWProxyBuilderFactory::get()->category()))
      {
         std::vector<edmplugin::PluginInfo> ac = edmplugin::PluginManager::get()->categoryToInfos().find(FWProxyBuilderFactory::get()->category())->second;
         for (auto &i : ac)
         {
            std::string pn = i.name_;

            std::string bType = pn.substr(0, pn.find_first_of('@'));
            edm::TypeWithDict modelType(*(iItem->modelType()->GetTypeInfo()));
            unsigned int distance = 1;

            std::string::size_type first = pn.find_first_of('@') + 1;
            std::string purpose = pn.substr(first, pn.find_last_of('@') - first);
            if (purpose != iItem->purpose())
            {
               continue;
            }

            if (!FWSimpleRepresentationChecker::inheritsFrom(modelType, bType, distance))
            {
               continue;
            }
            // printf("----///////////////////////////////////<<<<<<<<<<<<<  got [%s] match %s for item %s \n", pn.c_str(), bType.c_str(), iItem->modelType()->GetTypeInfo()->name() );

            auto builder = FWProxyBuilderFactory::get()->create(pn);
            addGraphicalProxyBuilder(collection, builder.release());
         }
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
      collection->GetItemList()->SetFillImpliedSelectedDelegate([&](REveDataItemList *collection, REveElement::Set_t &impSelSet) {
         this->FillImpliedSelected(collection, impSelSet);
      });
      collection->GetItemList()->SetItemsChangeDelegate([&](REveDataItemList *collection, const REveDataCollection::Ids_t &ids) {
         this->modelChanged(collection, ids);
      });
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
            tableBuilder->Build(collection, tablep, m_viewContext);
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
   builder->HaveSingleProduct();
   singleProduct = builder->CreateProduct("3D", m_viewContext);

   for (auto &ev : m_views)
   {
      if (ev->viewType() == "Table" || ev->viewType() == "Lego")
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

   for (auto &ev : m_views)
      ev->eventBegin();
}

void FW2EveManager::endEvent()
{
   for ( auto &i : m_builders) {
      i->Build();
   }

   for (auto &ev : m_views)
      ev->eventEnd();

   for (auto proxy : m_builders) {
      proxy->ScaleChanged();
   }

   m_acceptChanges = true;
}

//______________________________________________________________________________
void FW2EveManager::modelChanged(REveDataItemList* itemList, const REveDataCollection::Ids_t& ids) {
   if (!m_acceptChanges)
      return;
   
   for (auto proxy : m_builders) {
      if (proxy->Collection()->GetItemList() == itemList) {
         //printf("Model changes check proxy %s: \n", proxy->Type().c_str());
         proxy->ModelChanges(ids);
      }
   }
}
//______________________________________________________________________________
void FW2EveManager::FillImpliedSelected(REveDataItemList* itemList, REveElement::Set_t& impSelSet) {
   if (!m_acceptChanges)
      return;

    for (auto proxy : m_builders)
      {
         if (proxy->Collection()->GetItemList() == itemList)
         {
            proxy->FillImpliedSelected(impSelSet);
         }
      }
}
 
//______________________________________________________________________________
void
FW2EveManager::BuilderInfo::classType(std::string& typeName, bool& simple) const
{
   /*
   const std::string kSimple("simple#");
   //  simple = (m_name.substr(0,kSimple.size()) == kSimple);
   if (simple)
   {
      typeName = m_name.substr(kSimple.size(), m_name.find_first_of('@')-kSimple.size()-1);
   }
   else
   {
      typeName = m_name.substr(0, m_name.find_first_of('@')-1);
      }*/
    typeName = m_name.substr(0, m_name.find_first_of('@'));
}
//______________________________________________________________________________
FWTypeToRepresentations
FW2EveManager::supportedTypesAndRepresentations() const
{
   FWTypeToRepresentations returnValue;
   for(TypeToBuilder::const_iterator it = m_typeToBuilder.begin(), itEnd = m_typeToBuilder.end();
       it != itEnd;
       ++it) 
   {
      std::vector<BuilderInfo> blist = it->second;
      for (size_t bii = 0, bie = blist.size(); bii != bie; ++bii)
      {
         BuilderInfo &info = blist[bii];
          std::string name;
         bool isSimple = true;
         bool representsSubPart = false;
         unsigned int bitPackedViews = 0;
         bool FFOnly =false;
         info.classType(name, isSimple);
         if(isSimple) 
         {
            returnValue.add(std::make_shared<FWSimpleRepresentationChecker>(name, it->first,bitPackedViews,representsSubPart, FFOnly) );
         }
      }
   }

   return returnValue;
}
