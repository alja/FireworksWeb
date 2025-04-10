#include "FireworksWeb/Core/interface/FW3DView.h"
#include "FireworksWeb/Core/interface/FWEventAnnotation.h"
#include "FireworksWeb/Core/interface/FW3DViewGeometry.h"
#include "FireworksWeb/Core/interface/FWGeometry.h"
#include "FireworksWeb/Core/interface/Context.h"
#include "FireworksWeb/Core/interface/FWViewEnergyScale.h"

#include <ROOT/REveBoxSet.hxx>
#include <ROOT/REveGeoShape.hxx>
#include <ROOT/REveViewContext.hxx>
#include <ROOT/REveScene.hxx>
#include <ROOT/REveCalo.hxx>
#include <ROOT/REveTrans.hxx>
#include <ROOT/REveViewer.hxx>
#include <ROOT/REveScene.hxx>
#include <TGeoTube.h>

#include <ROOT/REveManager.hxx>

using namespace ROOT::Experimental;
//----------------------------------------------------------------
//----------------------------------------------------------------
//----------------------------------------------------------------
FW3DView::FW3DView(std::string vtype) : FWEveView(vtype),

      m_showMuonBarrel(this, "Show Muon Barrel", false),
      m_showMuonEndcap(this, "Show Muon Endcap", false),
      m_showPixelBarrel(this, "Show Pixel Barrel", false),
      m_showPixelEndcap(this, "Show Pixel Endcap", false),
      m_showTrackerBarrel(this, "Show Tracker Barrel", false),
      m_showTrackerEndcap(this, "Show Tracker Endcap", false),
      m_showEcalBarrel(this, "Show Ecal Barrel", false),
      m_showEventLabel(this, "Show EventLabel", false)
{
  m_geoScene = gEve->SpawnNewScene(Form("GeoScene %s", vtype.c_str()));
  m_viewer->AddScene(m_geoScene);

  //REveScene *os = gEve->SpawnNewScene("OverlayScene", "OverlayTitle");
  //m_viewer->AddScene(os);
  //os->SetIsOverlay(true);
  m_annotation = new FWEventAnnotation(m_eventScene); 

  m_ecalBarrel = new REveBoxSet("ecalBarrel");
  m_ecalBarrel->Reset(REveBoxSet::kBT_FreeBox, true, 0);
  m_ecalBarrel->SetMainColorPtr(new Color_t);
  m_ecalBarrel->UseSingleColor();
  m_ecalBarrel->SetMainColor(kAzure -1);
  m_ecalBarrel->SetMainTransparency(98);
  m_ecalBarrel->SetRnrSelf(false);
  geoScene()->AddElement(m_ecalBarrel);
}
FW3DView::~FW3DView(){}

void FW3DView::importContext(ROOT::Experimental::REveViewContext *)
{
  // Geometry
  auto b1 = new REveGeoShape("Barrel 1");
  float dr = 1;

  fireworks::Context *ctx = fireworks::Context::getInstance();
  b1->SetShape(new TGeoTube(ctx->caloR1(), ctx->caloR2() + dr, ctx->caloZ1()));
  b1->SetMainColor(kGray);
  b1->SetMainTransparency(95);
  m_geoScene->AddElement(b1);
  b1->SetRnrSelf(ctx->energyScale()->getDrawBarrel());

  auto m_geometry = new FW3DViewGeometry(*ctx);
  geoScene()->AddElement(m_geometry);

  m_showMuonBarrel.changed_.connect(std::bind(&FW3DViewGeometry::showMuonBarrel, m_geometry, std::placeholders::_1));
  m_showMuonEndcap.changed_.connect(std::bind(&FW3DViewGeometry::showMuonEndcap, m_geometry, std::placeholders::_1));
  m_showPixelBarrel.changed_.connect(std::bind(&FW3DViewGeometry::showPixelBarrel, m_geometry, std::placeholders::_1));
  m_showPixelEndcap.changed_.connect(std::bind(&FW3DViewGeometry::showPixelEndcap, m_geometry, std::placeholders::_1));
  m_showTrackerBarrel.changed_.connect(std::bind(&FW3DViewGeometry::showTrackerBarrel, m_geometry, std::placeholders::_1));
  m_showTrackerEndcap.changed_.connect(std::bind(&FW3DViewGeometry::showTrackerEndcap, m_geometry, std::placeholders::_1));
 // m_showEcalBarrel.changed_.connect(std::bind(&FW3DViewGeometry::showEcalBarrel, m_geometry, std::placeholders::_1));
  m_showEventLabel.changed_.connect(std::bind(&FW3DView::showEventLabel, this, std::placeholders::_1));

  // calo
  REveCaloData *data = ctx->getCaloData();
  m_calo3d = new REveCalo3D(data);
  m_calo3d->SetName("calo barrel");

  m_calo3d->SetBarrelRadius(ctx->caloR1(false));
  m_calo3d->SetEndCapPos(ctx->caloZ1(false));
  m_calo3d->SetAutoRange(false);
  m_calo3d->SetScaleAbs(true);
  m_eventScene->AddElement(m_calo3d);

  if (ctx->isOpendata()) {
      m_showEventLabel.set(true);
      // m_showMuonBarrel.set(true);
      m_viewer->SetAxesType(REveViewer::kAxesOrigin);
  }
}

REveCaloViz*
FW3DView::getEveCalo() const
{
  return dynamic_cast<REveCaloViz*>(m_calo3d);
}

void FW3DView::bgChanged(bool is_dark)
{
  viewer()->SetBlackBackground(is_dark);
  m_annotation->bgChanged(is_dark);
}

void FW3DView::showEventLabel(bool x)
{
  m_annotation->setLevel(x);
  StampObjProps();
}

void FW3DView::showEcalBarrel(bool x)
{
  m_showEcalBarrel.set(x);
  StampObjProps();
  if (x && m_ecalBarrel->GetPlex()->Size() == 0)
  {
    fireworks::Context *ctx = fireworks::Context::getInstance();
    const FWGeometry *geom = ctx->getGeom();
    std::vector<unsigned int> ids =
        geom->getMatchedIds(FWGeometry::Detector::Ecal, FWGeometry::SubDetector::PixelBarrel);
    m_ecalBarrel->Reset(REveBoxSet::kBT_FreeBox, true, ids.size());
    for (std::vector<unsigned int>::iterator it = ids.begin(); it != ids.end(); ++it)
    {
      const float *cor = geom->getCorners(*it);
      m_ecalBarrel->AddBox(cor);
    }
    m_ecalBarrel->RefitPlex();
    m_ecalBarrel->SetMainTransparency(90);
    m_ecalBarrel->StampObjProps();
  }
  if (m_ecalBarrel->GetRnrSelf() != x)
  {
    m_ecalBarrel->SetRnrSelf(x);
  }
}

void FW3DView::eventEnd()
{
   FWEveView::eventEnd();
   m_annotation->setEvent();
}

int FW3DView::WriteCoreJson(nlohmann::json &j, int rnr_offset)
{
  int ret = REveElement::WriteCoreJson(j, rnr_offset);
  j["eveViewId"] = viewer()->GetElementId();

  j["muonBarrel"] = (bool)m_showMuonBarrel.value();
  j["muonEndcap"] = (bool)m_showMuonEndcap.value();
  j["pixelBarrel"] = (bool)m_showPixelBarrel.value();
  j["pixelEndcap"] = (bool)m_showPixelEndcap.value();
  j["trackerBarrel"] = (bool)m_showTrackerBarrel.value();
  j["trackerEndcap"] = (bool)m_showTrackerEndcap.value();
  j["ecalBarrel"] = (bool)m_showEcalBarrel.value();
  j["showEventLabel"] =  (bool)m_showEventLabel.value();

  // std::cout << "FW3DView " << j.dump(3) << "\n";
  return ret;
}


  void FW3DView::setFrom(const FWConfiguration&)
  {
    // restore implemented
  }
  
  void FW3DView::addTo(FWConfiguration&) const
  {

    // no restore implemented
  }