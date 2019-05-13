
#include "TH2.h"
#include "TMath.h"

#include "Fireworks/Core/interface/fw3dlego_xbins.h"
#include "Fireworks2/Core/interface/Context.h"
#include "Fireworks2/Core/interface/FWBeamSpot.h"
#include "Fireworks2/Core/interface/FWMagField.h"

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
Context::Context():
  m_geom(nullptr),
  m_propagator(nullptr),
  m_trackerPropagator(nullptr),
  m_muonPropagator(nullptr),
  m_magField(nullptr),
  m_maxEt(1.f),
  m_maxEnergy(1.f)
{
   s_fwContext = this;
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

float Context::energyScale()
{
   // AMT temoprary here. should be in FWeViewContext
   return 10.f;
}

Context* Context::getInstance()
{
   return s_fwContext;
}
