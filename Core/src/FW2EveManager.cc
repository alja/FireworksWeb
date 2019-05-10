#include "Fireworks2/Core/interface/FW2EveManager.h"

#include <ROOT/REveManager.hxx>
#include "ROOT/REveTrackPropagator.hxx"
#include <ROOT/REveProjectionBases.hxx>
#include <ROOT/REveProjectionManager.hxx>
#include <ROOT/REveTableProxyBuilder.hxx>
#include <ROOT/REveGeoShape.hxx>
#include <TGeoTube.h>

#include <ROOT/REveScene.hxx>
#include <ROOT/REveViewer.hxx>
#include <ROOT/REveTableInfo.hxx>

//#include "Fireworks2/Core/src/FW2JetProxyBuilder.cc"
//#include "Fireworks2/Core/src/FW2JetProxyBuilder.cc"
//#include "Fireworks2/Core/src/FW2TableProxyBuilder.cc"

bool gRhoZView = true;

FW2EveManager::FW2EveManager():
   m_viewContext(0),
   m_mngRhoZ(0),
   m_acceptChanges(true)
{
      //view context
      float r = 300;
      float z = 300;
      auto prop = new REX::REveTrackPropagator();
      prop->SetMagFieldObj(new REX::REveMagFieldDuo(350, -3.5, 2.0));
      prop->SetMaxR(r);
      prop->SetMaxZ(z);
      prop->SetMaxOrbs(6);
      prop->IncRefCount();
      
      m_viewContext = new REX::REveViewContext();
      m_viewContext->SetBarrel(r, z);
      m_viewContext->SetTrackPropagator(prop);      

      
      // table specs
      auto tableInfo = new REX::REveTableViewInfo();
      tableInfo->table("Tracks").
         column("q", 1, "charge").
         column("pt", 1, "pt").
         column("eta", 3).
         column("phi", 3).
         column("d0", 5).
         column("d0Err", 5, "d0Error").
         column("dz", 5).
         column("dzErr", 5, "dzError").
         column("vx", 5).
         column("vy", 5).
         column("vz", 5).
         column("chi2", 3).
         // column("pixel hits", 1, "hitPattern().numberOfValidPixelHits()").
         // column("strip hits", 1, "hitPattern().numberOfValidStripHits()").
         column("ndof", 1);
      
      tableInfo->table("Jets").
         column("pT", 1, "pt").
         column("eta", 3).
         column("phi", 3).
         //  column("ECAL", 1, "p4().E() * emEnergyFraction()").
         //column("HCAL", 1, "p4().E() * energyFractionHadronic()").
         column("emf", 3, "emEnergyFraction");

      tableInfo->table("Muons").
         column("pT", 1, "pt").
         column("eta", 3).
         column("phi", 3).
   column("global", 1, "isGlobalMuon").
   column("tracker", 1, "isTrackerMuon").
   column("SA", 1, "isStandAloneMuon").
         column("q", 1, "charge");
  
      m_viewContext->SetTableViewInfo(tableInfo);

      
     createScenesAndViews();
}

//______________________________________________________________________________


void FW2EveManager::createScenesAndViews()
{

   // 3D
   m_scenes.push_back(REX::gEve->GetEventScene());

   // Geometry 
   auto b1 = new REX::REveGeoShape("Barrel 1");
   float dr = 3;
   b1->SetShape(new TGeoTube(m_viewContext->GetMaxR() , m_viewContext->GetMaxR() + dr, m_viewContext->GetMaxZ()));
   b1->SetMainColor(kCyan);
   REX::gEve->GetGlobalScene()->AddElement(b1);
      

   // RhoZ
   if (gRhoZView) {
      auto rhoZEventScene = REX::gEve->SpawnNewScene("RhoZ Scene","Projected");
      m_mngRhoZ = new REX::REveProjectionManager(REX::REveProjection::kPT_RhoZ);
      m_mngRhoZ->SetImportEmpty(true);
      auto rhoZView = REX::gEve->SpawnNewViewer("RhoZ View", "");
      rhoZView->AddScene(rhoZEventScene);
      m_scenes.push_back(rhoZEventScene);
         
      auto pgeoScene  = REX::gEve->SpawnNewScene("Projection Geometry","xxx");
      m_mngRhoZ->ImportElements(b1,pgeoScene );
      rhoZView->AddScene(pgeoScene);
   }

   // Table
   if (1) {
      auto tableScene  = REX::gEve->SpawnNewScene("Tables", "Tables");
         tableScene->AddElement(m_viewContext->GetTableViewInfo());
      auto tableView = REX::gEve->SpawnNewViewer("Table", "Table View");
      tableView->AddScene(tableScene);
      m_scenes.push_back(tableScene);
   }

}

//______________________________________________________________________________


   void FW2EveManager::registerCollection(REX::REveDataCollection* collection, REX::REveDataProxyBuilderBase* glBuilder, bool /*showTable*/)
{      
   // GL view types

   glBuilder->SetCollection(collection);
   glBuilder->SetHaveAWindow(true);
   for (REX::REveScene* scene : m_scenes) {
      REX::REveElement* product = glBuilder->CreateProduct(scene->GetTitle(), m_viewContext);
      if (strncmp(scene->GetCTitle(), "Table", 5) == 0) continue;
      if (!strncmp(scene->GetCTitle(), "Projected", 8)) {
         m_mngRhoZ->ImportElements(product, scene);
      }
      else {
         scene->AddElement(product);
      }
   }
   m_builders.push_back(glBuilder);
   glBuilder->Build();
   
  {
      // Table view types      {
     auto tableBuilder = new REX::REveTableProxyBuilder();
      tableBuilder->SetHaveAWindow(true);
      tableBuilder->SetCollection(collection);
      REX::REveElement* tablep = tableBuilder->CreateProduct("table-type", m_viewContext);

      auto tableMng =  m_viewContext->GetTableViewInfo();
      tableMng->AddDelegate([=](REX::ElementId_t elId) { tableBuilder->DisplayedCollectionChanged(elId); });

      //  printf("COMAPRE [%s], [%s] \n", collection->GetCName(), m_tableCollection.c_str());
      if (m_tableCollection.compare(collection->GetName()) == 0) {
          tableMng->SetDisplayedCollection(collection->GetElementId());
      }

      
      for (REX::REveScene* scene : m_scenes) {
         if (strncmp(scene->GetCTitle(), "Table", 5) == 0) {
            scene->AddElement(tablep);
            tableBuilder->Build(collection, tablep, m_viewContext );
         }
      }

      m_builders.push_back(tableBuilder);
   }
      
   //   collection->SetHandlerFunc([&] (REX::REveDataCollection* collection) { this->collectionChanged( collection ); });
   collection->SetHandlerFuncIds([&] (REX::REveDataCollection* collection, const REX::REveDataCollection::Ids_t& ids) { this->modelChanged( collection, ids ); });
}
//______________________________________________________________________________
void FW2EveManager::beginEvent()
{
   m_acceptChanges=false;
}

void FW2EveManager::endEvent()
{
   for ( auto &i : m_builders) {
      i->Build();
   }
   m_acceptChanges = true;
}
//______________________________________________________________________________

void FW2EveManager::modelChanged(REX::REveDataCollection* collection, const REX::REveDataCollection::Ids_t& ids) {
   if (!m_acceptChanges)
      return;
   for (auto proxy : m_builders) {
      if (proxy->Collection() == collection) {
         // printf("Model changes check proxy %s: \n", proxy->Type().c_str());
         proxy->ModelChanges(ids);
      }
   }
}
//______________________________________________________________________________

