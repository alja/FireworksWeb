#ifndef FireworksWeb_Core_FW2MuonProxyBuilder_h
#define FireworksWeb_Core_FW2MuonProxyBuilder_h

#include "DataFormats/MuonReco/interface/Muon.h"

#include "FireworksWeb/Muons/interface/FWMuonBuilder.h"
#include "FireworksWeb/Core/interface/FWProxyBuilderFactory.h"

#include "ROOT/REveTrack.hxx"
#include "ROOT/REveTrackPropagator.hxx"
#include "ROOT/REveViewContext.hxx"
#include "ROOT/REveDataSimpleProxyBuilderTemplate.hxx"
#include "TParticle.h"

class FWMuonProxyBuilder : public ROOT::Experimental::REveDataSimpleProxyBuilderTemplate<reco::Muon>
{
   FWMuonBuilder m_builder;
public:
   REGISTER_FWPB_METHODS();

  using REveDataSimpleProxyBuilderTemplate<reco::Muon>::BuildItem;
  virtual void BuildItem(const reco::Muon& iData, int /*idx*/, ROOT::Experimental::REveElement* iItemHolder, const ROOT::Experimental::REveViewContext* context) override
   {
      m_builder.buildMuon( this, &iData, iItemHolder, true, false );
   }
};


REGISTER_FW2PROXYBUILDER(FWMuonProxyBuilder, reco::Muon, "Muons");

#endif
