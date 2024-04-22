#include "FireworksWeb/Core/interface/FWEveView.h"
#include "FireworksWeb/Core/interface/Context.h"
#include "FireworksWeb/Core/interface/FWViewEnergyScale.h"
#include "FireworksWeb/Core/interface/FWTriggerTable.h"
#include "FireworksWeb/Core/interface/FW3DViewGeometry.h"
#include "FireworksWeb/Core/interface/FWGeometry.h"

#include <TGeoTube.h>
#include <TPad.h>
#include <TCanvas.h>
#include <TBufferJSON.h>
#include <THStack.h>
#include <TBase64.h>

#include <ROOT/REveManager.hxx>
#include <ROOT/REveViewer.hxx>
#include <ROOT/REveScene.hxx>
#include <ROOT/REveTrans.hxx>
#include <ROOT/REveCalo.hxx>
#include <ROOT/REvePointSet.hxx>
#include <ROOT/REveGeoShape.hxx>
#include <ROOT/REveViewContext.hxx>
#include <ROOT/REveTableInfo.hxx>
#include <ROOT/REveBoxSet.hxx>

using namespace ROOT::Experimental;

FWEveView::FWEveView(std::string vtype)
{
  SetNameTitle("FWView", "FWView");
  m_viewType = vtype;
  m_eventScene = gEve->SpawnNewScene(vtype.c_str(), vtype.c_str());
  m_viewer = gEve->SpawnNewViewer(vtype.c_str(), Form("%s View", vtype.c_str()));
  
  m_viewer->AddScene(m_eventScene);
}

FWEveView::~FWEveView()
{
}

void
FWEveView::eventBegin()
{
}

void FWEveView::eventEnd()
{
  setupEnergyScale();
}

fireworks::Context*
FWEveView::context(){
  return fireworks::Context::getInstance();
}

void FWEveView::voteCaloMaxVal() {
  REveCaloViz* calo = getEveCalo();
  if (calo)
    context()->voteMaxEtAndEnergy(calo->GetData()->GetMaxVal(true), calo->GetData()->GetMaxVal(false));
}

void FWEveView::setupEnergyScale()
{
  FWViewEnergyScale *energyScale = context()->energyScale();
  // printf("setupEnergyScale %s >> scale name %s\n", typeName().c_str(), energyScale->name().c_str());
  voteCaloMaxVal();

  float maxVal = context()->getMaxEnergyInEvent(energyScale->getPlotEt());
  energyScale->updateScaleFactors(maxVal);

  // configure REveCaloViz
  REveCaloViz *calo = getEveCalo();
  if (calo)
  {
    calo->SetPlotEt(energyScale->getPlotEt());
    /*
    if (FWViewType::isLego(typeId())) {
      float f = energyScale->getScaleFactorLego();
      calo->SetMaxValAbs(TMath::Pi() / f);
    } else {
      float f = energyScale->getScaleFactor3D();
      calo->SetMaxValAbs(100 / f);
    }
    calo->ElementChanged();
*/
    float f = energyScale->getScaleFactor3D();
    calo->SetMaxValAbs(100 / f);
    calo->StampObjProps();
  }
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

//----------------------------------------------------------------
FWTriggerTableView::FWTriggerTableView(std::string vtype) : FWEveView(vtype)
{
   m_triggerTable = new FWTriggerTable(context()->getCurrentEvent());
   m_eventScene->AddElement(m_triggerTable);
}

void FWTriggerTableView::eventEnd()
{
  m_triggerTable->readTriggerData();
}

//----------------------------------------------------------------
//----------------------------------------------------------------
//----------------------------------------------------------------
FW3DView::FW3DView(std::string vtype) : FWEveView(vtype),

      m_showMuonBarrel(this, "Show Muon Barrel", false),
      m_showMuonEndcap(this, "Show Muon Endcap", false),
      m_showPixelBarrel(this, "Show Pixel Barrel", false),
      m_showPixelEndcap(this, "Show Pixel Endcap", false),
      m_showTrackerBarrel(this, "Show Tracker Barrel", false),
      m_showTrackerEndcap(this, "Show Tracker Endcap", false)
{
  m_geoScene = gEve->SpawnNewScene(Form("GeoScene %s", vtype.c_str()));
  m_viewer->AddScene(m_geoScene);


  m_ecalBarrel = new REveBoxSet("ecalBarrel");
  m_ecalBarrel->SetMainColorPtr(new Color_t);
  m_ecalBarrel->UseSingleColor();
  m_ecalBarrel->SetMainColor(kAzure -1);
  m_ecalBarrel->SetMainTransparency(98);
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

  // calo
  REveCaloData *data = ctx->getCaloData();
  m_calo3d = new REveCalo3D(data);
  m_calo3d->SetName("calo barrel");

  m_calo3d->SetBarrelRadius(ctx->caloR1(false));
  m_calo3d->SetEndCapPos(ctx->caloZ1(false));
  m_calo3d->SetAutoRange(false);
  m_calo3d->SetScaleAbs(true);
  m_calo3d->SetMaxTowerH(300);
  m_eventScene->AddElement(m_calo3d);
}

REveCaloViz*
FW3DView::getEveCalo() const
{
  return dynamic_cast<REveCaloViz*>(m_calo3d);
}

void FW3DView::showEcalBarrel(bool x)
{
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

// std::cout << "FW3DView " << j.dump(3) << "\n";
  return ret;
}


//----------------------------------------------------------------
//----------------------------------------------------------------
//----------------------------------------------------------------
FWLegoView::FWLegoView(std::string vtype) : FWEveView(vtype)
{
  m_lego = new REvePointSet("Calo Lego");
  m_eventScene->AddElement(m_lego);
  m_viewer->SetName("Lego");
  m_pad = new TCanvas("LegoPad", "Lego Pad Tit", 800, 400);
  m_pad->SetMargin(0, 0, 0, 0);
}
FWLegoView::~FWLegoView() {}

void FWLegoView::eventEnd()
{
  m_pad->Modified(kTRUE);
  fireworks::Context *ctx = fireworks::Context::getInstance();
  ctx->getCaloData()->GetStack()->Draw();
  TString json(TBufferJSON::ToJSON(m_pad));
  m_lego->SetTitle(TBase64::Encode(json).Data());
  m_lego->SetMainColor(kWhite);
  m_lego->StampObjProps();
}

void FWLegoView::importContext(ROOT::Experimental::REveViewContext *)
{
  fireworks::Context *ctx = fireworks::Context::getInstance();
  REveCaloDataHist *data = ctx->getCaloData();
  m_pad->GetListOfPrimitives()->Add(data->GetStack());
}
