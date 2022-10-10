#include "FireworksWeb/Core/interface/FWEveView.h"
#include "FireworksWeb/Core/interface/Context.h"
#include "FireworksWeb/Core/interface/FWViewEnergyScale.h"
#include "FireworksWeb/Core/interface/FWTriggerTable.h"

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
FW3DView::FW3DView(std::string vtype) : FWEveView(vtype)
{
  m_geoScene = gEve->SpawnNewScene(Form("GeoScene %s", vtype.c_str()));
  m_viewer->AddScene(m_geoScene);
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
 // m_geoScene->AddElement(b1);

  // calo
  REveCaloData *data = ctx->getCaloData();
  m_calo3d = new REveCalo3D(data);
  m_calo3d->SetName("calo barrel");

  m_calo3d->SetBarrelRadius(ctx->caloR1(false));
  m_calo3d->SetEndCapPos(ctx->caloZ1(false));
  m_calo3d->SetFrameTransparency(99);
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
