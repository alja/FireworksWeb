#include "Fireworks2/Core/interface/FW2EveManager.h"
#include "Fireworks2/Core/interface/Context.h"
#include "FWCore/PluginManager/interface/PluginFactory.h"

#include <ROOT/REveManager.hxx>
#include "ROOT/REveTrackPropagator.hxx"
#include <ROOT/REveProjectionBases.hxx>
#include <ROOT/REveProjectionManager.hxx>
#include <ROOT/REveTableProxyBuilder.hxx>
#include <ROOT/REveGeoShape.hxx>
#include <ROOT/REveDataProxyBuilderBase.hxx>
#include <TGeoTube.h>

#include <ROOT/REveScene.hxx>
#include <ROOT/REveViewer.hxx>
#include <ROOT/REveTableInfo.hxx>

using namespace ROOT::Experimental;
bool gRhoZView = true;

FW2EveManager::FW2EveManager():
   m_viewContext(0),
   m_mngRhoZ(0),
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

      
      // table specs
      auto tableInfo = new REveTableViewInfo("cmsShowTableInfo");
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

      tableInfo->table("MET").
         column("et", 1).
         column("phi", 3).
         column("sumEt", 1).
         column("mEtSig", 3);

      tableInfo->table("Electrons").
         column("pT", 1, "pt").
         column("eta", 3).
         column("phi", 3).
         column("E/p", 3, "eSuperClusterOverP").
         column("H/E", 3, "hadronicOverEm").
         column("dei",3, "deltaEtaSuperClusterTrackAtVtx" ).
         column("dpi", 3, "deltaPhiSuperClusterTrackAtVtx()").
         column("charge", 0, "charge").
         column("isPF", 0, "isPF()").
         column("sieie", 3, "sigmaIetaIeta");
      //         column("isNotConv", 1, "passConversionVeto");


      table("CSCSegment").
      column("chi2", 0, "chi2");
      
      m_viewContext->SetTableViewInfo(tableInfo);

      
     createScenesAndViews();

   
     readPlugins();
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
        } else 
      */
      {
         m_mngRhoZ->GetProjection()->AddPreScaleEntry(0, fireworks::Context::caloR1(), 1.0);
         m_mngRhoZ->GetProjection()->AddPreScaleEntry(1, 310, 1.0);
         m_mngRhoZ->GetProjection()->AddPreScaleEntry(0, 370, 0.2);
         m_mngRhoZ->GetProjection()->AddPreScaleEntry(1, 580, 0.2);
      }
      
      auto rhoZView = gEve->SpawnNewViewer("RhoZ View", "RhoZ");
      rhoZView->AddScene(rhoZEventScene);
      m_scenes.push_back(rhoZEventScene);
         
      auto pgeoScene  = gEve->SpawnNewScene("Projection Geometry","xxx");
      m_mngRhoZ->ImportElements(b1,pgeoScene );
      rhoZView->AddScene(pgeoScene);
   }

   // Table
   if (1) {
      auto tableScene  = gEve->SpawnNewScene("Tables", "Tables");
      tableScene->AddElement(m_viewContext->GetTableViewInfo());
      auto tableView = gEve->SpawnNewViewer("Table", "Table View");
      tableView->AddScene(tableScene);
      m_scenes.push_back(tableScene);
   }

}

//______________________________________________________________________________


void FW2EveManager::readPlugins() {
   std::cout << "READ PLUGINS ------------------------------\n";
   /*
 std::vector<edmplugin::PluginInfo> available = edmplugin::PluginFactory<ROOT::Experimental::REveDataProxyBuilderBase*()>::get()->available();
 for (auto &x : available) {
    std::cout << " ____ plugin ___ " << x.name_ << std::endl;
 }
   */
}
//______________________________________________________________________________


   void FW2EveManager::registerCollection(REveDataCollection* collection, REveDataProxyBuilderBase* glBuilder, bool /*showTable*/)
{      
   // GL view types

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
   m_builders.push_back(glBuilder);
   glBuilder->Build();

  {
      // Table view types      {
     auto tableBuilder = new REveTableProxyBuilder();
      tableBuilder->SetHaveAWindow(true);
      tableBuilder->SetCollection(collection);
      REveElement* tablep = tableBuilder->CreateProduct("table-type", m_viewContext);

      auto tableMng =  m_viewContext->GetTableViewInfo();
      tableMng->AddDelegate([=](ElementId_t elId) { tableBuilder->DisplayedCollectionChanged(elId); });

      bool buildTable = false;
      if (m_tableCollection.compare(collection->GetName()) == 0) {
          tableMng->SetDisplayedCollection(collection->GetElementId());
          buildTable = true;
      }

      
      for (REveScene* scene : m_scenes) {
         if (strncmp(scene->GetCTitle(), "Table", 5) == 0) {
            scene->AddElement(tablep);
            if (buildTable) {
               tableBuilder->Build(collection, tablep, m_viewContext );
            }
         }
      }

      m_builders.push_back(tableBuilder);
   }
      
   //   collection->SetHandlerFunc([&] (REveDataCollection* collection) { this->collectionChanged( collection ); });
   collection->SetHandlerFuncIds([&] (REveDataCollection* collection, const REveDataCollection::Ids_t& ids) { this->modelChanged( collection, ids ); });
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

void FW2EveManager::modelChanged(REveDataCollection* collection, const REveDataCollection::Ids_t& ids) {
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

