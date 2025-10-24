#include <iostream>
#include "ROOT/REveManager.hxx"
#include "ROOT/REveScene.hxx"
#include "ROOT/REveText.hxx"
#include "ROOT/REveViewer.hxx"

#include "FireworksWeb/Core/interface/FWEventAnnotation.h"
#include "FireworksWeb/Core/interface/Context.h"
#include "FireworksWeb/Core/interface/FWColorManager.h"
#include "FireworksWeb/Core/interface/BuilderUtils.h"

#include "DataFormats/FWLite/interface/Event.h"
#include "FWCore/Common/interface/EventBase.h"


FWEventAnnotation::FWEventAnnotation(ROOT::Experimental::REveElement* h) :
m_level(0),
m_holder(h) 
{
}

FWEventAnnotation::~FWEventAnnotation() {}


using namespace ROOT::Experimental;
//______________________________________________________________________________

void FWEventAnnotation::setLevel(long x) {
  if (x != m_level) {
    m_level = x;
  }
  updateOverlayText();
}

void FWEventAnnotation::setEvent() { updateOverlayText(); }


void FWEventAnnotation::bgChanged(bool is_dark) {
  if (m_eveText) {
    m_eveText->SetTextColor(fireworks::Context::getInstance()->colorManager()->foreground());
  }
}


void FWEventAnnotation::updateOverlayText() {
  std::string info = "CMS Experiment at LHC, CERN";

  const edm::EventBase* event = fireworks::Context::getInstance()->getCurrentEvent();
  if (event && m_level) {
    assertEveText();
    info += "\nData recorded: ";
    info += fireworks::getLocalTime(*event);
    info += "\nRun/Event: ";
    info += std::to_string(event->id().run());
    info += " / ";
    info += std::to_string(event->id().event());
    if (m_level > 1) {
      info += "\nLumi section: ";
      info += event->luminosityBlock();
    }
    if (m_level > 2) {
      info += "\nOrbit/Crossing: ";
      info += event->orbitNumber();
      info += " / ";
      info += event->bunchCrossing();
    }
    m_eveText->SetRnrSelf(true);
    m_eveText->SetText(info);
  }

   if (m_eveText && m_level == 0)
   {
      m_eveText->SetRnrSelf(false);
   }

  std::cout << "event overlay info " << info << "  ------\n";

}

REveText* FWEventAnnotation::assertEveText()
{
  if (! m_eveText) {
    m_eveText = new REveText("Annotation");
    m_eveText->SetText("Event Info");


    m_eveText->SetMode(1);
    m_eveText->SetPosition(REveVector(0.005, 0.99, 0.000001));
    m_eveText->SetFontSize(0.015);
    m_eveText->SetFont("LiberationSans-Bold");
    m_eveText->SetTextColor(fireworks::Context::getInstance()->colorManager()->foreground());
    m_eveText->SetDrawFrame(false);

    m_holder->AddElement(m_eveText);
  }

  return m_eveText;
}

/*
//______________________________________________________________________________

void FWEventAnnotation::addTo(FWConfiguration& iTo) const {
  std::stringstream s;
  s << fTextSize;
  iTo.addKeyValue("EventInfoTextSize", FWConfiguration(s.str()));

  std::stringstream x;
  x << fPosX;
  iTo.addKeyValue("EventInfoPosX", FWConfiguration(x.str()));

  std::stringstream y;
  y << fPosY;
  iTo.addKeyValue("EventInfoPosY", FWConfiguration(y.str()));
}

void FWEventAnnotation::setFrom(const FWConfiguration& iFrom) {
  const FWConfiguration* value;

  value = iFrom.valueForKey("EventInfoTextSize");
  if (value)
    fTextSize = atof(value->value().c_str());

  value = iFrom.valueForKey("EventInfoPosX");
  if (value)
    fPosX = atof(value->value().c_str());

  value = iFrom.valueForKey("EventInfoPosY");
  if (value)
    fPosY = atof(value->value().c_str());
}
*/