
#include <ROOT/REveManager.hxx>
#include "ROOT/REveTrackPropagator.hxx"
#include <ROOT/REveProjectionBases.hxx>
#include <ROOT/REveProjectionManager.hxx>
#include <ROOT/REveTableProxyBuilder.hxx>
#include <ROOT/REveGeoShape.hxx>
#include <ROOT/REveCalo.hxx>
#include <ROOT/REveDataProxyBuilderBase.hxx>
#include <TGeoTube.h>

#include <ROOT/REveScene.hxx>
#include <ROOT/REveViewer.hxx>
#include <ROOT/REveTableInfo.hxx>

#include "FWCore/PluginManager/interface/PluginFactory.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "Fireworks2/Core/interface/FWSimpleRepresentationChecker.h"
#include "Fireworks2/Core/interface/FWProxyBuilderFactory.h"
#include "Fireworks2/Core/interface/FW2EveManager.h"
#include "Fireworks2/Core/interface/FWTableViewManager.h"
#include "Fireworks2/Core/interface/Context.h"
#include "Fireworks2/Core/interface/FWBeamSpot.h"
#include "Fireworks2/Core/interface/FWEventItem.h"

using namespace ROOT::Experimental;
bool gRhoZView = true;
bool gTable = true;

FW2EveManager::FW2EveManager(FWTableViewManager* iTableMng):
   m_viewContext(0),
   m_mngRhoZ(0),
   m_tableManager(iTableMng),
   m_acceptChanges(true)
{
      //view context
      float r = 300;
      float z = 300;
      auto prop = new REveTrackPropagator();
      prop->SetMagFieldObj(new REveMagFieldDuo(350, -3.5, 2.0));
      prop->SetMaxR(r);
      prop->SetMaxZ(z);
      prop->SetMaxOrbs(6);
      prop->IncRefCount();

      m_viewContext = new REveViewContext();
      m_viewContext->SetBarrel(r, z);
      m_viewContext->SetTrackPropagator(prop);
      m_viewContext->SetTableViewInfo(m_tableManager->getTableInfo());

      initTypeToBuilder();
      createScenesAndViews();
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

   // 3D
   m_scenes.push_back(gEve->GetEventScene());

   // Geometry
   auto b1 = new REveGeoShape("Barrel 1");
   float dr = 3;

   fireworks::Context* ctx = fireworks::Context::getInstance();
   b1->SetShape(new TGeoTube(ctx->caloR1(), ctx->caloR2() + dr, ctx->caloZ1()));
   b1->SetMainColor(kCyan);
   gEve->GetGlobalScene()->AddElement(b1);
 
 
  REveCaloData* data = ctx->getCaloData();
  REveCalo3D* calo = new REveCalo3D(data);
  calo->SetName("calo barrel");

  calo->SetBarrelRadius(ctx->caloR1(false));
  calo->SetEndCapPos(ctx->caloZ1(false));
  calo->SetFrameTransparency(80);
  calo->SetAutoRange(false);
  calo->SetScaleAbs(true);
  gEve->GetEventScene()->AddElement(calo);

   // RhoZ
   if (gRhoZView) {
      auto rhoZEventScene = gEve->SpawnNewScene("RhoZ Scene","RhoZ");
      rhoZEventScene->SetTitle("RhoZ");
      m_mngRhoZ = new REveProjectionManager(REveProjection::kPT_RhoZ);
      m_mngRhoZ->SetImportEmpty(true);

      /*
        if ( id == FWViewType::kRhoPhi || id == FWViewType::kRhoPhiPF) {
        m_projMgr->GetProjection()->AddPreScaleEntry(0, fireworks::Context::caloR1(), 1.0);
        m_projMgr->GetProjection()->AddPreScaleEntry(0, 300, 0.2);
        } 
      */
      {
         m_mngRhoZ->GetProjection()->AddPreScaleEntry(0, fireworks::Context::caloR1(), 1.0);
         m_mngRhoZ->GetProjection()->AddPreScaleEntry(1, 310, 1.0);
         m_mngRhoZ->GetProjection()->AddPreScaleEntry(0, 370, 0.2);
         m_mngRhoZ->GetProjection()->AddPreScaleEntry(1, 580, 0.2);
      }

      auto rhoZView = gEve->SpawnNewViewer("RhoZ View", "RhoZ");
      rhoZView->AddScene(rhoZEventScene);
      m_mngRhoZ->ImportElements( calo, rhoZEventScene);
      m_scenes.push_back(rhoZEventScene);

      auto pgeoScene  = gEve->SpawnNewScene("Projection Geometry","xxx");
      m_mngRhoZ->ImportElements(b1,pgeoScene );
      rhoZView->AddScene(pgeoScene);
   }

   // Table
   if (gTable) {
      auto tableScene  = gEve->SpawnNewScene("Tables", "Tables");
      tableScene->AddElement(m_viewContext->GetTableViewInfo());
      auto tableView = gEve->SpawnNewViewer("Table", "Table View");
      tableView->AddScene(tableScene);
      m_scenes.push_back(tableScene);
   }

}

//______________________________________________________________________________
void FW2EveManager::newItem(FWEventItem* iItem)
{
   printf("============================================= newItem %s \n", iItem->name() );
   try {
      if(edmplugin::PluginManager::get()->categoryToInfos().end()!=edmplugin::PluginManager::get()->categoryToInfos().find(FWProxyBuilderFactory::get()->category()))
      {
         std::vector<edmplugin::PluginInfo> ac = edmplugin::PluginManager::get()->categoryToInfos().find(FWProxyBuilderFactory::get()->category())->second;
         for (auto &i : ac) {
            std::string pn = i.name_;

            std::string bType =  pn.substr(0, pn.find_first_of('@'));
            edm::TypeWithDict modelType( *(iItem->modelType()->GetTypeInfo()));
            unsigned int distance = 1;

            std::string::size_type first =pn.find_first_of('@')+1;
            std::string purpose = pn.substr(first, pn.find_last_of('@')-first);
            if (purpose != iItem->purpose())
            {
               continue;
            }

            if (!FWSimpleRepresentationChecker::inheritsFrom(modelType, bType,distance))
            {
               continue;
            }
            // printf("----///////////////////////////////////<<<<<<<<<<<<<  got [%s] match %s for item %s \n", pn.c_str(), bType.c_str(), iItem->modelType()->GetTypeInfo()->name() );


            auto builder = FWProxyBuilderFactory::get()->create(pn);
            registerGraphicalProxy(iItem->getCollection(), builder.release());
            //  }
         }
         registerCollection(iItem->getCollection(), true);
      }
   }
   catch (const cms::Exception& iE){
      std::cout << iE << std::endl;
   }
}

//______________________________________________________________________________
void FW2EveManager::registerCollection(REveDataCollection* collection, bool showTable = true)
{

   auto tableInfo =  m_viewContext->GetTableViewInfo();
   if (gTable && showTable)
   {
      // Table view types      {
      auto tableBuilder = new REveTableProxyBuilder();
      tableBuilder->SetHaveAWindow(true);
      tableBuilder->SetCollection(collection);
      REveElement* tablep = tableBuilder->CreateProduct("table-type", m_viewContext);

      tableInfo->AddDelegate([=]() { tableBuilder->ConfigChanged(); });

      bool buildTable = false;
      if (m_tableManager->getDisplayedCollection().compare(collection->GetName()) == 0) {
         tableInfo->SetDisplayedCollection(collection->GetElementId());
         printf("FOUBND TABLE \n");
         buildTable = true;
      }

      for (REveScene* scene : m_scenes) {
         if (strncmp(scene->GetCTitle(), "Table", 5) == 0) {
            scene->AddElement(tablep);
            if (buildTable) {
               m_tableManager->checkExpressionsForType(collection->GetItemClass());
               tableBuilder->Build(collection, tablep, m_viewContext );
            }
         }
      }

      m_builders.push_back(tableBuilder);
   }
   
     auto tableEntries =  tableInfo->RefTableEntries(collection->GetItemClass()->GetName());
      int N  = TMath::Min(int(tableEntries.size()), 3);
      for (int t = 0; t < N; t++) {
         auto te = tableEntries[t];
         collection->GetItemList()->AddTooltipExpression(te.fName, te.fExpression);
      }


      collection->GetItemList()->SetFillImpliedSelectedDelegate([&] (REveDataItemList* collection, REveElement::Set_t& impSelSet)
                                    {
                                       this->FillImpliedSelected( collection,  impSelSet);
                                    });
                                    
      collection->GetItemList()->SetItemsChangeDelegate([&] (REveDataItemList* collection, const REveDataCollection::Ids_t& ids)
                                    {
                                       this->modelChanged( collection, ids );
                                    });

}

//______________________________________________________________________________
void FW2EveManager::registerGraphicalProxy(REveDataCollection* collection, REveDataProxyBuilderBase* glBuilder)
{
   // GL view types
   printf("registerCollection %s\n", collection->GetCName());
   glBuilder->SetCollection(collection);
   glBuilder->SetHaveAWindow(true);
   for (REveScene* scene : m_scenes) {
      if (strncmp(scene->GetCTitle(), "Table", 5) == 0) continue;
      if (!strncmp(scene->GetCTitle(), "Rho", 3)) {
         REveElement* product = glBuilder->CreateProduct(scene->GetTitle(), m_viewContext);
         m_mngRhoZ->ImportElements(product, scene);
      }
      else {
         REveElement* product = glBuilder->CreateProduct("3D", m_viewContext);
         scene->AddElement(product);
      }
   }
   printf("proxy single product %d \n", glBuilder->HaveSingleProduct());
   m_builders.push_back(glBuilder);
   glBuilder->Build();
}
//______________________________________________________________________________
void FW2EveManager::beginEvent()
{
   m_acceptChanges=false;

   auto bs = fireworks::Context::getInstance()->getBeamSpot();
   REveVector c(bs->x0(), bs->y0(), bs->z0());
   m_mngRhoZ->GetProjection()->SetCenter(c);
}

void FW2EveManager::endEvent()
{
   for ( auto &i : m_builders) {
      i->Build();
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
   printf("FW2EveManager FillImpliedSelected\n");
 
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
   // needed for add collection GUI
   FWTypeToRepresentations returnValue;
   const static std::string kFullFrameWorkPBExtension = "FullFramework";
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
