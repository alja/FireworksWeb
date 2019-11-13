// -*- C++ -*-
//
// Package:     Core
// Class  :     FWGenParticleProxyBuilder
//
/**\class FWGenParticleProxyBuilder 

   Description: <one line class summary>

   Usage:
    <usage>

 */
//
// Original Author:
//         Created:  Thu Dec  6 18:01:21 PST 2007
//

#include "ROOT/REveTrack.hxx"
#include "ROOT/REveTrackPropagator.hxx"
#include "ROOT/REveViewContext.hxx"
#include "ROOT/REveDataSimpleProxyBuilderTemplate.hxx"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/HepMCCandidate/interface/GenParticleFwd.h"


#include "Fireworks2/Core/interface/FWProxyBuilderFactory.h"
#include "Fireworks2/Core/interface/Context.h"
#include "Fireworks2/Core/interface/FWMagField.h"
#include "Fireworks2/Candidates/interface/CandidateUtils.h"
#include "Fireworks2/Tracks/interface/estimate_field.h"
using namespace ROOT::Experimental;

class FWGenParticleProxyBuilder : public REveDataSimpleProxyBuilderTemplate<reco::GenParticle>
{
public:
   REGISTER_FWPB_METHODS();

   using REveDataSimpleProxyBuilderTemplate<reco::GenParticle>::Build;
   virtual void Build(const reco::GenParticle& iData, int /*idx*/, REveElement* iItemHolder, const REveViewContext* vc) override
   {
      auto context = fireworks::Context::getInstance();
      REveTrackPropagator* propagator = context->getTrackPropagator();
      REveTrack* trk = fireworks::prepareCandidate(iData, propagator);
      trk->MakeTrack();
      SetupAddElement(trk, iItemHolder);
   }
};


REGISTER_FW2PROXYBUILDER(FWGenParticleProxyBuilder, reco::GenParticle, "GenParticles");




// -*- C++ -*-
//
// Package:     Core
// Class  :     FWGenParticleProxyBuilder
//
/**\class FWGenParticleProxyBuilder 

   Description: <one line class summary>

   Usage:
    <usage>

 */
//
// Original Author:
//         Created:  Thu Dec  6 18:01:21 PST 2007
//

/*
class FWGenParticleProxyBuilder : public FWSimpleProxyBuilderTemplate<reco::GenParticle> {
public:
  FWGenParticleProxyBuilder() {}
  ~FWGenParticleProxyBuilder() override {}

  // ---------- member functions ---------------------------
  REGISTER_PROXYBUILDER_METHODS();

private:
  FWGenParticleProxyBuilder(const FWGenParticleProxyBuilder&) = delete;  // stop default

  const FWGenParticleProxyBuilder& operator=(const FWGenParticleProxyBuilder&) = delete;  // stop default

  using FWSimpleProxyBuilderTemplate<reco::GenParticle>::build;
  void build(const reco::GenParticle& iData,
             unsigned int iIndex,
             TEveElement& oItemHolder,
             const FWViewContext*) override;
};

//______________________________________________________________________________

void FWGenParticleProxyBuilder::build(const reco::GenParticle& iData,
                                      unsigned int iIndex,
                                      TEveElement& oItemHolder,
                                      const FWViewContext*) {
  TEveTrack* trk = fireworks::prepareCandidate(iData, context().getTrackPropagator());
  trk->MakeTrack();
  setupAddElement(trk, &oItemHolder);
}

REGISTER_FWPROXYBUILDER(FWGenParticleProxyBuilder,
                        reco::GenParticle,
                        "GenParticles",
                        FWViewType::kAll3DBits | FWViewType::kAllRPZBits);
*/
