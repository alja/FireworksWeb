#include "FireworksWeb/Core/interface/FWEveView.h"
#include "FireworksWeb/Core/interface/Context.h"

#include <ROOT/REveManager.hxx>
#include <ROOT/REveViewer.hxx>
#include <ROOT/REveScene.hxx>
#include <ROOT/REveTrans.hxx>
#include <ROOT/REveCalo.hxx>
#include <ROOT/REveGeoShape.hxx>
#include <ROOT/REveTableInfo.hxx>
#include <ROOT/REveViewContext.hxx>

#include <TGeoTube.h>

using namespace ROOT::Experimental;

FWEveView::FWEveView(std::string vtype)
{
  m_viewType = vtype;
  m_eventScene = gEve->SpawnNewScene(vtype.c_str(), vtype.c_str());
  m_viewer = gEve->SpawnNewViewer(vtype.c_str(), Form("%s View", vtype.c_str()));
  m_viewer->AddScene(m_eventScene);
}

FWEveView::~FWEveView()
{
}

void
FWEveView::eventBegin(){}

void
FWEveView::eventEnd(){}

fireworks::Context*
FWEveView::context(){
  return fireworks::Context::getInstance();
}

//----------------------------------------------------------------
//----------------------------------------------------------------
//----------------------------------------------------------------
FW3DView::FW3DView(std::string vtype) : FWEveView(vtype)
{
  m_geoScene = gEve->SpawnNewScene(Form("GeoScene %s", vtype.c_str()));
  m_viewer->AddScene(m_geoScene);
}

void FW3DView::importContext(ROOT::Experimental::REveViewContext *)
{
  // Geometry
  auto b1 = new REveGeoShape("Barrel 1");
  float dr = 3;

  fireworks::Context *ctx = fireworks::Context::getInstance();
  b1->SetShape(new TGeoTube(ctx->caloR1(), ctx->caloR2() + dr, ctx->caloZ1()));
  b1->SetMainColor(kCyan);
  m_geoScene->AddElement(b1);

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
  m_eventScene->AddElement(calo);
}
//----------------------------------------------------------------
//----------------------------------------------------------------
//----------------------------------------------------------------
FWTableView::FWTableView(std::string vtype) : FWEveView(vtype)
{
}

void FWTableView::importContext(ROOT::Experimental::REveViewContext* vc)
{
  m_eventScene->AddElement(vc->GetTableViewInfo());
}
