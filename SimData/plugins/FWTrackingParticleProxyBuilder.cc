/*
 *  FWTrackingParticleProxyBuilder.cc
 *  FWorks
 *
 *  Created by Ianna Osborne on 9/9/10.
 *
 */

#include "FireworksWeb/Core/interface/Context.h"
// #include "FireworksWeb/Core/interface/FWEventItem.h"
#include "FireworksWeb/Core/interface/FWGeometry.h"
#include "FireworksWeb/Core/interface/FWProxyBuilderFactory.h"
#include "FireworksWeb/Core/interface/FWProxyBuilderConfiguration.h"
#include "FireworksWeb/Core/interface/FWParameters.h"

#include "SimDataFormats/TrackingAnalysis/interface/TrackingParticle.h"

#include "ROOT/REveTrack.hxx"
#include "ROOT/REveDataSimpleProxyBuilderTemplate.hxx"


using namespace ROOT::Experimental;
class FWTrackingParticleProxyBuilder : public REveDataSimpleProxyBuilderTemplate<TrackingParticle>
{
public:
   REGISTER_FWPB_METHODS();

   using REveDataSimpleProxyBuilderTemplate<TrackingParticle>::BuildItem;
   virtual void BuildItem(const TrackingParticle& iData, int /*idx*/, REveElement* iItemHolder, const REveViewContext* vc) override
   {     
      REveRecTrack t;
      t.fBeta = 1.0;
      t.fP = REveVector(iData.px(), iData.py(), iData.pz());
      t.fV = REveVector(iData.vx(), iData.vy(), iData.vz());
      t.fSign = iData.charge();

      fireworks::Context *context = fireworks::Context::getInstance();
      REveTrack* track = new REveTrack(&t, context->getTrackPropagator());
      if (t.fSign == 0)
        track->SetLineStyle(7);

      track->MakeTrack();
      SetupAddElement(track, iItemHolder);
    }
};


REGISTER_FW2PROXYBUILDER(FWTrackingParticleProxyBuilder, TrackingParticle, "TrackingParticles");
