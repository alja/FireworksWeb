#ifndef FireworksWeb_Tracks_FW2TrackTrackingRecHitProxyBuilder_h
#define FireworksWeb_Tracks_FW2TrackTrackingRecHitProxyBuilder_h

#include "ROOT/REveTrack.hxx"
#include "ROOT/REveTrackPropagator.hxx"
#include "ROOT/REveViewContext.hxx"
#include "ROOT/REveDataSimpleProxyBuilderTemplate.hxx"
#include "TParticle.h"

#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackerRecHit2D/interface/SiStripRecHit2D.h"
#include "DataFormats/TrackerRecHit2D/interface/SiStripRecHit1D.h"
#include "DataFormats/TrackingRecHit/interface/TrackingRecHit.h"

#include "FireworksWeb/Core/interface/FWProxyBuilderFactory.h"
#include "FireworksWeb/Core/interface/FWGeometry.h"
#include "FireworksWeb/Core/interface/Context.h"
#include "FireworksWeb/Core/interface/fwLog.h"
#include "FireworksWeb/Tracks/interface/TrackUtils.h"

using namespace ROOT::Experimental;

class FWTrackTrackingRecHitProxyBuilder : public REveDataSimpleProxyBuilderTemplate<reco::Track>
{
public:
   REGISTER_FWPB_METHODS();

   using REveDataSimpleProxyBuilderTemplate<reco::Track>::BuildItem;
   virtual void BuildItem(const reco::Track &iData, int /*idx*/, REveElement *iItemHolder, const REveViewContext *vc) override
   {
      const FWGeometry *geom = fireworks::Context::getInstance()->getGeom();
      REvePointSet *pointSet = new REvePointSet;
      pointSet->SetMarkerSize(2);
      SetupAddElement(pointSet, iItemHolder);

      for (trackingRecHit_iterator it = iData.recHitsBegin(), itEnd = iData.recHitsEnd(); it != itEnd; ++it)
      {

         auto rechitRef = *it;
         const TrackingRecHit *rechit = &(*rechitRef);

         if (rechit->isValid())
         {
            unsigned int rawid = rechit->geographicalId().rawId();

            if (!geom->contains(rawid))
            {
               fwLog(fwlog::kError) << "failed get geometry for detid: " << rawid << std::endl;
            }

            LocalPoint pos(0.0, 0.0, 0.0);
            if (const SiStripRecHit2D *hit = dynamic_cast<const SiStripRecHit2D *>(rechit))
            {
               if (hit->hasPositionAndError())
               {
                  pos = rechit->localPosition();
               }
            }
            else if (const SiStripRecHit1D *hit = dynamic_cast<const SiStripRecHit1D *>(rechit))
            {
               if (hit->hasPositionAndError())
               {
                  pos = rechit->localPosition();
               }
            }

            float localPos[3] = {pos.x(), pos.y(), pos.z()};
            float globalPos[3];
            geom->localToGlobal(rawid, localPos, globalPos);
            pointSet->SetNextPoint(globalPos[0], globalPos[1], globalPos[2]);
         }
      }
   }
};

REGISTER_FW2PROXYBUILDER(FWTrackTrackingRecHitProxyBuilder, reco::Track, "Track Tracking RecHits");
#endif

