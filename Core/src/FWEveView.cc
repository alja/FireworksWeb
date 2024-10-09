#include "FireworksWeb/Core/interface/FWEveView.h"
#include "FireworksWeb/Core/interface/Context.h"
#include "FireworksWeb/Core/interface/FWViewEnergyScale.h"
#include "FireworksWeb/Core/interface/FWWebTriggerTable.h"

//#include <TGeoTube.h>
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
#include <ROOT/REveViewContext.hxx>
#include <ROOT/REvePointSet.hxx>
#include <ROOT/REveTableInfo.hxx>
#include <ROOT/REveText.hxx>

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
FWWebTriggerTableView::FWWebTriggerTableView(std::string vtype) : FWEveView(vtype)
{
   m_triggerTable = new FWWebTriggerTable(context()->getCurrentEvent());
   m_eventScene->AddElement(m_triggerTable);
}

void FWWebTriggerTableView::eventEnd()
{
  m_triggerTable->readTriggerData();
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
