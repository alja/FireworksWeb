
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
  FW3DView(vtype),
  m_shiftOrigin(this, "Shift origin to beam-spot", false),
  m_fishEyeDistortion(this, "Distortion", 0., 0., 100.),
  m_fishEyeR(this, "FixedRadius", (double)fireworks::Context::caloR1(), 0.0, 150.0),

  m_caloDistortion(this, "Calo compression", 1.0, 0.01, 10.),
  m_muonDistortion(this, "Muon compression", 0.2, 0.01, 10.),
  // m_showProjectionAxes(this, "Show projection axis", false),
  // m_projectionAxesLabelSize(this, "Projection axis label size", 0.015, 0.001, 0.2),
  m_compressMuon(this, "Compress detectors", false),
  m_showPixelBarrel(this, "Show Pixel Barrel", false),
  m_showPixelEndcap(this, "Show Pixel Endcap", false),
  m_showTrackerBarrel(this, "Show Tracker Barrel", false),
  m_showTrackerEndcap(this, "Show Tracker Endcap", false),
  m_showRpcEndcap(this, "Show RPC Endcap", false),
  m_showGEM(this, "Show GEM", false),
  m_showME0(this, "Show ME0", false),
  m_includeEndcaps(this, "Include Endcaps", true)
{
  viewer()->SetCameraType(REveViewer::kCameraOrthoXOY);

  REveProjection::EPType_e projType = (vtype == "RhoZ") ? REveProjection::kPT_RhoZ : REveProjection::kPT_RPhi;
  m_projMgr = new REveProjectionManager(projType);
  m_projMgr->IncDenyDestroy();
  m_projMgr->SetImportEmpty(kTRUE);

  if (projType == REveProjection::kPT_RPhi) {
    m_projMgr->GetProjection()->AddPreScaleEntry(0, fireworks::Context::caloR1(), 1.0);
    m_projMgr->GetProjection()->AddPreScaleEntry(0, 300, 0.6);
  } else {
    m_projMgr->GetProjection()->AddPreScaleEntry(0, fireworks::Context::caloR1(), 1.0);
    m_projMgr->GetProjection()->AddPreScaleEntry(1, 310, 1.0);
    m_projMgr->GetProjection()->AddPreScaleEntry(0, 370, 0.6);
    m_projMgr->GetProjection()->AddPreScaleEntry(1, 580, 0.4);
  }
  
  // doCompression(true); // signal should be connected with m_compressMuon
  // doFishEyeDistortion();
}

FWRPZView::~FWRPZView(){}

void FWRPZView::eventBegin()
{
  FWEveView::eventBegin();
  auto bs = context()->getBeamSpot();
  REveVector c(bs->x0(), bs->y0(), bs->z0());
  m_projMgr->GetProjection()->SetCenter(c);


  // HACK!!! temporary solution, geometry can be initialized after file load
  // windows at the moment are created before oarse of cofiguration
  if (!m_geoInitialized)
  {
    auto odepth = m_projMgr->GetCurrentDepth();
    m_projMgr->SetCurrentDepth(-20);

    REveElement* sg = m_geometryList->initStdGeoElements(viewType());

  REveProjected* proj = *m_geometryList->RefProjecteds().begin();
  proj->GetManager()->SubImportElements(sg, proj->GetProjectedAsElement());

    //REveElement *p = m_projMgr->ImportElements(m_geometryList);
    //geoScene()->AddElement(p);

    m_projMgr->SetCurrentDepth(odepth);
    m_geoInitialized = true;
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
  m_calo->SetScaleAbs(true);
  m_calo->SetAutoRange(false);
  m_calo->SetBarrelRadius(ctx->caloR1(false));
  m_calo->SetEndCapPos(ctx->caloZ1(false));

  // this is an import of an empty container
  // this will change as the view configuration is part of FWConfiguration
  m_geometryList = new FWRPZViewGeometry(*ctx);
  m_geometryList->IncDenyDestroy();
  //ROOT::Experimental::gEve->GetGlobalScene()->AddElement(m_geometryList);
  m_projMgr->ImportElements(m_geometryList, geoScene());

  m_showPixelBarrel.changed_.connect(
      std::bind(&FWRPZViewGeometry::showPixelBarrel, m_geometryList, std::placeholders::_1));
  m_showPixelEndcap.changed_.connect(
      std::bind(&FWRPZViewGeometry::showPixelEndcap, m_geometryList, std::placeholders::_1));
  m_showTrackerBarrel.changed_.connect(
      std::bind(&FWRPZViewGeometry::showTrackerBarrel, m_geometryList, std::placeholders::_1));
  m_showTrackerEndcap.changed_.connect(
      std::bind(&FWRPZViewGeometry::showTrackerEndcap, m_geometryList, std::placeholders::_1));
  m_showRpcEndcap.changed_.connect(std::bind(&FWRPZViewGeometry::showRpcEndcap, m_geometryList, std::placeholders::_1));
  m_showGEM.changed_.connect(std::bind(&FWRPZViewGeometry::showGEM, m_geometryList, std::placeholders::_1));
  m_showME0.changed_.connect(std::bind(&FWRPZViewGeometry::showME0, m_geometryList, std::placeholders::_1));
  m_showMtdBarrel.changed_.connect(std::bind(&FWRPZViewGeometry::showMtdBarrel, m_geometryList, std::placeholders::_1));
  m_showMtdEndcap.changed_.connect(std::bind(&FWRPZViewGeometry::showMtdEndcap, m_geometryList, std::placeholders::_1));
}

REveCaloViz *
FWRPZView::getEveCalo() const
{
  return dynamic_cast<REveCaloViz*>(m_calo);
}

void FWRPZView::doFishEyeDistortion() {
  static const float s_distortF = 0.001;

  REveProjection* p = m_projMgr->GetProjection();
  if (p->GetDistortion() != m_fishEyeDistortion.value() * s_distortF)
    p->SetDistortion(m_fishEyeDistortion.value() * s_distortF);
  if (p->GetFixR() != m_fishEyeR.value())
    p->SetFixR(m_fishEyeR.value());
}

void FWRPZView::doPreScaleDistortion() {
  if (m_projMgr->GetProjection()->GetType() == REveProjection::kPT_RPhi)
  {
    m_projMgr->GetProjection()->ChangePreScaleEntry(0, 1, m_caloDistortion.value());
    m_projMgr->GetProjection()->ChangePreScaleEntry(0, 2, m_muonDistortion.value());
  } else {
    m_projMgr->GetProjection()->ChangePreScaleEntry(0, 1, m_caloDistortion.value());
    m_projMgr->GetProjection()->ChangePreScaleEntry(0, 2, m_muonDistortion.value());
    m_projMgr->GetProjection()->ChangePreScaleEntry(1, 1, m_caloDistortion.value());
    m_projMgr->GetProjection()->ChangePreScaleEntry(1, 2, m_muonDistortion.value());
  }
  m_projMgr->UpdateName();
}

void FWRPZView::doCompression(bool flag) {
  m_projMgr->GetProjection()->SetUsePreScale(flag);
}

void FWRPZView::bgChanged(bool is_dark)
{
  viewer()->SetBlackBackground(is_dark);
}


void FWRPZView::setEtaRng(bool x)
{
  if (m_viewType == "RPhi") {
    m_includeEndcaps.set(x);
    double eta_range = fireworks::Context::caloMaxEta();
    if (!m_includeEndcaps.value())
    eta_range = fireworks::Context::caloTransEta();
    
    m_calo->SetEta(-eta_range, eta_range);
    FWEveView::setupEnergyScale();
  }
}

int FWRPZView::WriteCoreJson(nlohmann::json &j, int rnr_offset)
{
  int ret = REveElement::WriteCoreJson(j, rnr_offset);
  j["eveViewId"] = viewer()->GetElementId();

  j["pixelBarrel"] = (bool)m_showPixelBarrel.value();
  j["pixelEndcap"] = (bool)m_showPixelEndcap.value();
  j["trackerBarrel"] = (bool)m_showTrackerBarrel.value();
  j["trackerEndcap"] = (bool)m_showTrackerEndcap.value();
  j["rpcEndcap"] = (bool)m_showRpcEndcap.value();
  j["gem"] = (bool)m_showGEM.value();
  j["me0"] = (bool)m_showME0.value();
  j["rpcMtdcap"] = (bool)m_showMtdEndcap.value();
// std::cout << "FW3DView " << j.dump(3) << "\n";
  return ret;
}
