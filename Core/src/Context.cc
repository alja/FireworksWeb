
#include "TH2.h"
#include "TMath.h"
#include "ROOT/REveCaloData.hxx"
#include "ROOT/REveManager.hxx"
#include "ROOT/REveScene.hxx"
#include "FireworksWeb/Core/interface/FW2Main.h"
#include "FireworksWeb/Core/interface/fw3dlego_xbins.h"
#include "FireworksWeb/Core/interface/Context.h"
#include "FireworksWeb/Core/interface/FWBeamSpot.h"
#include "FireworksWeb/Core/interface/FWMagField.h"
#include "FireworksWeb/Core/interface/fw3dlego_xbins.h"

#include "FireworksWeb/Core/interface/FWViewEnergyScale.h"
#include "FireworksWeb/Core/interface/FWEventItemsManager.h"

using namespace fireworks;
using namespace ROOT::Experimental;

Context* Context::s_fwContext = nullptr;

const float Context::s_caloTransEta = 1.479;
const float Context::s_caloTransAngle = 2*atan(exp(-s_caloTransEta));

// simplified
const float Context::s_caloZ  = 290;
const float Context::s_caloR  = s_caloZ*tan(s_caloTransAngle);

// calorimeter offset between REveCalo and outlines (used by proxy builders)
const float Context::s_caloOffR = 10;
const float Context::s_caloOffZ = s_caloOffR/tan(s_caloTransAngle);



//
// constructors and destructor
//
Context::Context(const FW2Main* iMain):
  m_main(iMain)
{
   s_fwContext = this;
   m_energyScale = new FWViewEnergyScale("global");
}


Context::~Context()
{
}

void
Context::initEveElements()
{
   m_magField = new FWMagField();
   m_beamSpot = new FWBeamSpot();

   float propagatorOffR = 5;
   float propagatorOffZ = propagatorOffR*caloZ1(false)/caloR1(false);

   // common propagator, helix stepper
   m_propagator = new REveTrackPropagator();
   m_propagator->SetMagFieldObj(m_magField, false);
   m_propagator->SetMaxR(caloR2()-propagatorOffR);
   m_propagator->SetMaxZ(caloZ2()-propagatorOffZ);
   m_propagator->SetDelta(0.01);
   //   m_propagator->SetProjTrackBreaking(m_commonPrefs->getProjTrackBreaking());
   // m_propagator->SetRnrPTBMarkers(m_commonPrefs->getRnrPTBMarkers());
   m_propagator->IncDenyDestroy();
   // tracker propagator
   m_trackerPropagator = new REveTrackPropagator();
   m_trackerPropagator->SetStepper( REveTrackPropagator::kRungeKutta );
   m_trackerPropagator->SetMagFieldObj(m_magField, false);
   m_trackerPropagator->SetDelta(0.01);
   m_trackerPropagator->SetMaxR(caloR1()-propagatorOffR);
   m_trackerPropagator->SetMaxZ(caloZ2()-propagatorOffZ);
   //   m_trackerPropagator->SetProjTrackBreaking(m_commonPrefs->getProjTrackBreaking());
   // m_trackerPropagator->SetRnrPTBMarkers(m_commonPrefs->getRnrPTBMarkers());
   m_trackerPropagator->IncDenyDestroy();
   // muon propagator
   m_muonPropagator = new REveTrackPropagator();
   m_muonPropagator->SetStepper( REveTrackPropagator::kRungeKutta );
   m_muonPropagator->SetMagFieldObj(m_magField, false);
   m_muonPropagator->SetDelta(0.05);
   m_muonPropagator->SetMaxR(850.f);
   m_muonPropagator->SetMaxZ(1100.f);
   // m_muonPropagator->SetProjTrackBreaking(m_commonPrefs->getProjTrackBreaking());
   // m_muonPropagator->SetRnrPTBMarkers(m_commonPrefs->getRnrPTBMarkers());
   m_muonPropagator->IncDenyDestroy();

{
    m_caloData = new REveCaloDataHist();
    gEve->GetGlobalScene()->AddElement(m_caloData);
    m_caloData->IncDenyDestroy();

    // Phi range is always in the (-Pi, Pi) without a shift.
    // Set wrap to false for the optimisation on TEveCaloData::GetCellList().
    m_caloData->SetWrapTwoPi(false);

    m_caloData->SetSelector(new REveCaloDataSelector());

    Bool_t status = TH1::AddDirectoryStatus();
    TH1::AddDirectory(kFALSE);  //Keeps histogram from going into memory
    TH2F* dummy =
        new TH2F("background", "background", fw3dlego::xbins_n - 1, fw3dlego::xbins, 72, -1 * TMath::Pi(), TMath::Pi());


      dummy->GetXaxis()->SetTitle("#eta");
      dummy->GetYaxis()->SetLabelSize(1);
      dummy->GetYaxis()->SetTitle("#varphi");
      dummy->GetZaxis()->SetLabelSize(1);

    TH1::AddDirectory(status);
    Int_t sliceIndex = m_caloData->AddHistogram(dummy);
    (m_caloData)->RefSliceInfo(sliceIndex).Setup("background", 0., 0);

  }
}

void
Context::voteMaxEtAndEnergy(float et, float energy) const
{
   m_maxEt     = TMath::Max(et    , m_maxEt    );
   m_maxEnergy = TMath::Max(energy, m_maxEnergy);
}

void
Context::resetMaxEtAndEnergy() const
{
   // should not be zero, problems with infinte bbox

   m_maxEnergy = 1.f;
   m_maxEt     = 1.f;
}

float
Context::getMaxEnergyInEvent(bool isEt) const
{
   return isEt ?  m_maxEt : m_maxEnergy;
}

//
// static member functions
//

float Context::caloR1(bool offset)
{
   return offset ? (s_caloR -offset) :s_caloR;
}

float Context::caloR2(bool offset)
{

   return offset ? (s_caloR -offset) :s_caloR;
}
float Context::caloZ1(bool offset)
{
   return offset ? (s_caloZ -offset) :s_caloZ;
}

float Context::caloZ2(bool offset)
{
   return offset ? (s_caloZ -offset) :s_caloZ;
}

float Context::caloTransEta()
{
   return s_caloTransEta;
}

float Context::caloTransAngle()
{
   return s_caloTransAngle;
}

double Context::caloMaxEta()
{
   return fw3dlego::xbins_hf[fw3dlego::xbins_hf_n -1];
}
const fwlite::Event* Context::getCurrentEvent() const
{
  return m_main->getCurrentEvent();
}

const FWEventItem *Context::getItemForCollection(const ROOT::Experimental::REveDataCollection *c)
{
   return m_main->getEventItemsManager()->find(c);
}
/*
float Context::energyScale()
{
   // AMT temoprary here. should be in FWeViewContext
   return 10.f;
}*/

Context* Context::getInstance()
{
   return s_fwContext;
}
