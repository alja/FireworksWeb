#ifndef Fireworks2_Core_FW2MuonProxyBuilder_h
#define Fireworks2_Core_FW2MuonProxyBuilder_h

#include "Fireworks2/Muons/interface/FWMuonBuilder.h"
#include "DataFormats/MuonReco/interface/Muon.h"


#include "ROOT/REveTrack.hxx"
#include "ROOT/REveTrackPropagator.hxx"
#include "ROOT/REveViewContext.hxx"
#include "ROOT/REveDataSimpleProxyBuilderTemplate.hxx"
#include "TParticle.h"

class FWMuonProxyBuilder : public REX::REveDataSimpleProxyBuilderTemplate<reco::Muon>
{
   FWMuonBuilder m_builder;
   
   using REveDataSimpleProxyBuilderTemplate<reco::Muon>::Build;
   virtual void Build(const reco::Muon& iData, REX::REveElement* iItemHolder, const REX::REveViewContext* context)
   {
      m_builder.buildMuon( this, &iData, iItemHolder, true, false );
   }
};


#endif
