
#include "FireworksWeb/Core/interface/FWRPZView.h"
#include "FireworksWeb/Core/interface/FWGeometry.h"
#include "FireworksWeb/Core/interface/Context.h"
#include "FireworksWeb/Core/interface/FWBeamSpot.h"
#include "FireworksWeb/Core/interface/FWRPZViewGeometry.h"

#include <ROOT/REveManager.hxx>
#include <ROOT/REveProjectionManager.hxx>
#include <ROOT/REveProjectionBases.hxx>
#include <ROOT/REveTrans.hxx>
#include <ROOT/REveCalo.hxx>
#include <ROOT/REveGeoShape.hxx>
#include <ROOT/REveScene.hxx>
#include <ROOT/REveViewer.hxx>

#include <TGeoTube.h>

using namespace ROOT::Experimental;


//-------------------------------------------------------------------------------------------------------
FWRPZView::FWRPZView(std::string vtype):
FW3DView(vtype)
{
  viewer()->SetCameraType(REveViewer::kCameraOrthoXOY);

  REveProjection::EPType_e projType = (vtype == "RhoZ") ? REveProjection::kPT_RhoZ : REveProjection::kPT_RPhi;
  m_projMgr = new REveProjectionManager(projType);
  m_projMgr->IncDenyDestroy();
  m_projMgr->SetImportEmpty(kTRUE);
}

FWRPZView::~FWRPZView(){}

void FWRPZView::eventBegin()
{
  FWEveView::eventBegin();
  auto bs = context()->getBeamSpot();
  REveVector c(bs->x0(), bs->y0(), bs->z0());
  m_projMgr->GetProjection()->SetCenter(c);

  // geometry can be initialized after file load
  if (!m_geometryList)
  {
    m_geometryList = new FWRPZViewGeometry();
    m_geometryList->IncDenyDestroy();
    m_geometryList->initStdGeoElements(viewType());
    
    m_projMgr->SetCurrentDepth(-1);
    m_projMgr->ImportElements(m_geometryList, geoScene());
  }
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
FWRPZView::importContext(ROOT::Experimental::REveViewContext *)
{
  auto ctx = context();
  // calo
  REveCaloData *data = ctx->getCaloData();
  REveCalo3D *calo = new REveCalo3D(data);
  calo->SetName("calo barrel");

  //calo->SetFrameTransparency(80);
  //calo->SetAutoRange(false);
  //calo->SetScaleAbs(true);
  m_calo = static_cast<REveCalo2D *>(m_projMgr->ImportElements(calo, eventScene()));
  m_calo->SetMaxTowerH(100);
  m_calo->SetScaleAbs(true);
  m_calo->SetAutoRange(false);
  m_calo->SetBarrelRadius(ctx->caloR1(false));
  m_calo->SetEndCapPos(ctx->caloZ1(false));
}

REveCaloViz *
FWRPZView::getEveCalo() const
{
  return dynamic_cast<REveCaloViz*>(m_calo);
}
