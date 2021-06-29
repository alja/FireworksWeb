
#include "FireworksWeb/Core/interface/FWRPZView.h"
#include "FireworksWeb/Core/interface/FWGeometry.h"
#include "FireworksWeb/Core/interface/Context.h"
#include "FireworksWeb/Core/interface/FWBeamSpot.h"

#include <ROOT/REveManager.hxx>
#include <ROOT/REveProjectionManager.hxx>
#include <ROOT/REveProjectionBases.hxx>
#include <ROOT/REveTrans.hxx>
#include <ROOT/REveCalo.hxx>
#include <ROOT/REveGeoShape.hxx>
#include <ROOT/REveScene.hxx>

#include <TGeoTube.h>

#include "DataFormats/MuonDetId/interface/DTChamberId.h"
#include "DataFormats/MuonDetId/interface/CSCDetId.h"
#include "DataFormats/MuonDetId/interface/RPCDetId.h"
#include "DataFormats/MuonDetId/interface/GEMDetId.h"
#include "DataFormats/MuonDetId/interface/ME0DetId.h"

using namespace ROOT::Experimental;

namespace {
struct ProjGeo {
   ProjGeo() { m_geom = fireworks::Context::getInstance()->getGeom();}
   
   const FWGeometry* m_geom {nullptr};
   REveGeoShape* makeShape(double min_rho, double max_rho, double min_z, double max_z) {
      REveTrans t;
      t(1, 1) = 1;
      t(1, 2) = 0;
      t(1, 3) = 0;
      t(2, 1) = 0;
      t(2, 2) = 1;
      t(2, 3) = 0;
      t(3, 1) = 0;
      t(3, 2) = 0;
      t(3, 3) = 1;
      t(1, 4) = 0;
      t(2, 4) = (min_rho + max_rho) / 2;
      t(3, 4) = (min_z + max_z) / 2;

      REveGeoShape* shape = new REveGeoShape;
      shape->SetTransMatrix(t.Array());

      shape->SetRnrSelf(kTRUE);
      shape->SetRnrChildren(kTRUE);
      TGeoBBox* box = new TGeoBBox(0, (max_rho - min_rho) / 2, (max_z - min_z) / 2);
      shape->SetShape(box);

      return shape;
   }

   //______________________________________________________________________________

   void estimateProjectionSizeDT(
                                 const FWGeometry::GeomDetInfo& info, float& min_rho, float& max_rho, float& min_z, float& max_z) {
      // we will test 5 points on both sides ( +/- z)
      float local[3], global[3];

      float dX = info.shape[1];
      float dY = info.shape[2];
      float dZ = info.shape[3];

      local[0] = 0;
      local[1] = 0;
      local[2] = dZ;
      m_geom->localToGlobal(info, local, global);
      estimateProjectionSize(global, min_rho, max_rho, min_z, max_z);

      local[0] = dX;
      local[1] = dY;
      local[2] = dZ;
      m_geom->localToGlobal(info, local, global);
      estimateProjectionSize(global, min_rho, max_rho, min_z, max_z);

      local[0] = -dX;
      local[1] = dY;
      local[2] = dZ;
      m_geom->localToGlobal(info, local, global);
      estimateProjectionSize(global, min_rho, max_rho, min_z, max_z);

      local[0] = dX;
      local[1] = -dY;
      local[2] = dZ;
      m_geom->localToGlobal(info, local, global);
      estimateProjectionSize(global, min_rho, max_rho, min_z, max_z);

      local[0] = -dX;
      local[1] = -dY;
      local[2] = dZ;
      m_geom->localToGlobal(info, local, global);
      estimateProjectionSize(global, min_rho, max_rho, min_z, max_z);

      local[0] = 0;
      local[1] = 0;
      local[2] = -dZ;
      m_geom->localToGlobal(info, local, global);
      estimateProjectionSize(global, min_rho, max_rho, min_z, max_z);

      local[0] = dX;
      local[1] = dY;
      local[2] = -dZ;
      m_geom->localToGlobal(info, local, global);
      estimateProjectionSize(global, min_rho, max_rho, min_z, max_z);

      local[0] = -dX;
      local[1] = dY;
      local[2] = -dZ;
      m_geom->localToGlobal(info, local, global);
      estimateProjectionSize(global, min_rho, max_rho, min_z, max_z);

      local[0] = dX;
      local[1] = -dY;
      local[2] = -dZ;
      m_geom->localToGlobal(info, local, global);
      estimateProjectionSize(global, min_rho, max_rho, min_z, max_z);

      local[0] = -dX;
      local[1] = -dY;
      local[2] = -dZ;
      m_geom->localToGlobal(info, local, global);
      estimateProjectionSize(global, min_rho, max_rho, min_z, max_z);
   }

   void estimateProjectionSize(
                               const float* global, float& min_rho, float& max_rho, float& min_z, float& max_z) {
      double rho = sqrt(global[0] * global[0] + global[1] * global[1]);
      if (min_rho > rho)
         min_rho = rho;
      if (max_rho < rho)
         max_rho = rho;
      if (min_z > global[2])
         min_z = global[2];
      if (max_z < global[2])
         max_z = global[2];
   }
}; // end struc
}// namespace


//-------------------------------------------------------------------------------------------------------
FWRPZView::FWRPZView(std::string vtype):
FW3DView(vtype)
{
  REveProjection::EPType_e projType = (vtype == "RhoZ") ? REveProjection::kPT_RhoZ : REveProjection::kPT_RPhi;
  m_projMgr = new REveProjectionManager(projType);
  m_projMgr->IncDenyDestroy();
  m_projMgr->SetImportEmpty(kTRUE);

  ProjGeo helper;
  for (Int_t iWheel = -2; iWheel <= 2; ++iWheel)
  {
      for (Int_t iStation = 1; iStation <= 4; ++iStation)
      {
          float min_rho(1000), max_rho(0), min_z(2000), max_z(-2000);

          // This will give us a quarter of DTs
          // which is enough for our projection
          for (Int_t iSector = 1; iSector <= 4; ++iSector)
          {
              DTChamberId id(iWheel, iStation, iSector);
              unsigned int rawid = id.rawId();
              FWGeometry::IdToInfoItr det = context()->getGeom()->find(rawid);
              if (det != context()->getGeom()->mapEnd())
              {
                  helper.estimateProjectionSizeDT(*det, min_rho, max_rho, min_z, max_z);
              }
          }
          if (min_rho > max_rho || min_z > max_z)
              continue;
          REveElement *se = helper.makeShape(min_rho, max_rho, min_z, max_z);
          se->SetMainColor(kRed);
          m_projMgr->ImportElements(se, m_geoScene);
          se = helper.makeShape(-max_rho, -min_rho, min_z, max_z);
          se->SetMainColor(kRed);
          m_projMgr->ImportElements(se, m_geoScene);
      }
  }
}

FWRPZView::~FWRPZView(){}

void
FWRPZView::eventBegin() {
   auto bs = context()->getBeamSpot();
   REveVector c(bs->x0(), bs->y0(), bs->z0());
   m_projMgr->GetProjection()->SetCenter(c);
}

void
FWRPZView::eventEnd() {
  FWEveView::eventEnd();
}

void 
FWRPZView::importElements(REveElement* iChildren, float layer, REveElement *iProjectedParent)
{
  m_projMgr->SetCurrentDepth(layer);
  m_projMgr->ImportElements(iChildren, iProjectedParent);
}


void
FWRPZView::importContext(ROOT::Experimental::REveViewContext*){
    auto ctx = context();
    // calo
    REveCaloData *data = ctx->getCaloData();
    REveCalo3D *calo = new REveCalo3D(data);
    calo->SetName("calo barrel");

    calo->SetBarrelRadius(ctx->caloR1(false));
    calo->SetEndCapPos(ctx->caloZ1(false));
    calo->SetFrameTransparency(80);
    calo->SetAutoRange(false);
    calo->SetScaleAbs(true);
    calo->SetMaxTowerH(300);
    m_projMgr->ImportElements(calo, m_eventScene);
}
