#ifndef FireworksWeb_Tracks_FW2TrackProxyBuilder_h
#define FireworksWeb_Tracks_FW2TrackProxyBuilder_h

#include "ROOT/REveTrack.hxx"
#include "ROOT/REveTrackPropagator.hxx"
#include "ROOT/REveViewContext.hxx"
#include "ROOT/REveDataSimpleProxyBuilderTemplate.hxx"
#include "TParticle.h"
#include "DataFormats/TrackReco/interface/Track.h"


#include "FireworksWeb/Core/interface/FWProxyBuilderFactory.h"
#include "FireworksWeb/Core/interface/Context.h"
#include "FireworksWeb/Core/interface/FWMagField.h"
#include "FireworksWeb/Tracks/interface/TrackUtils.h"
#include "FireworksWeb/Tracks/interface/estimate_field.h"
using namespace ROOT::Experimental;

class FWTrackProxyBuilder : public REveDataSimpleProxyBuilderTemplate<reco::Track>
{
public:
   REGISTER_FWPB_METHODS();

   using REveDataSimpleProxyBuilderTemplate<reco::Track>::BuildItem;
   virtual void BuildItem(const reco::Track& iData, int /*idx*/, REveElement* iItemHolder, const REveViewContext* vc) override
   {
      auto context = fireworks::Context::getInstance();

      if( context->getField()->getSource() == FWMagField::kNone ) {
         if( fabs( iData.eta() ) < 2.0 && iData.pt() > 0.5 && iData.pt() < 30 ) {
            double estimate = fw::estimate_field( iData, true );
            if( estimate >= 0 ) context->getField()->guessField( estimate );
         }
      }

      REveTrackPropagator* propagator = ( !iData.extra().isAvailable() ) ?  context->getTrackerTrackPropagator() : context->getTrackPropagator();
      REveTrack* trk = fireworks::prepareTrack( iData, propagator );
      trk->MakeTrack();

      SetupAddElement(trk, iItemHolder);
   }
};


REGISTER_FW2PROXYBUILDER(FWTrackProxyBuilder, reco::Track, "Tracks");
#endif

