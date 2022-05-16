// -*- C++ -*-
//
// Package:     Tracks FWTrajectorySeedProxyBuilder
// Class  :     
//
//

#include "ROOT/REvePointSet.hxx"
#include "ROOT/REveLine.hxx"
#include "ROOT/REveStraightLineSet.hxx"
#include "ROOT/REveCompound.hxx"
#include "ROOT/REveDataSimpleProxyBuilderTemplate.hxx"
#include "ROOT/REveGeoShape.hxx"
#include "FireworksWeb/Core/interface/FWProxyBuilderFactory.h"
#include "FireworksWeb/Core/interface/FWEventItem.h"
#include "FireworksWeb/Core/interface/FWGeometry.h"
#include "FireworksWeb/Core/interface/fwLog.h"
#include "FireworksWeb/Core/interface/Context.h"
#include "FireworksWeb/Core/interface/FWEventItem.h"
#include "FireworksWeb/Tracks/interface/TrackUtils.h"

#include "DataFormats/TrajectorySeed/interface/TrajectorySeed.h"
#include "DataFormats/TrackerRecHit2D/interface/SiPixelRecHit.h"
#include "DataFormats/SiPixelCluster/interface/SiPixelCluster.h"

using namespace ROOT::Experimental;

class FWTrajectorySeedProxyBuilder : public REveDataSimpleProxyBuilderTemplate<TrajectorySeed>
{
public:
  REGISTER_FWPB_METHODS();
  using REveDataSimpleProxyBuilderTemplate<TrajectorySeed>::BuildItem;
  virtual void BuildItem(const TrajectorySeed &iData, int /*idx*/, REveElement *iItemHolder, const REveViewContext *vc) override
  {
    REvePointSet *pointSet = new REvePointSet;
    pointSet->SetMarkerSize(3);
    REveLine *line = new REveLine;
    REveStraightLineSet *lineSet = new REveStraightLineSet;

    for (auto const &hit : iData.recHits())
    {
      unsigned int id = hit.geographicalId();
      const FWGeometry *geom = fireworks::Context::getInstance()->getGeom();
      const float *pars = geom->getParameters(id);
      const SiPixelRecHit *rh = dynamic_cast<const SiPixelRecHit *>(&hit);
      // std::cout << id << "id "<< 	std::endl;
      if (rh)
      {
        const SiPixelCluster *itc = rh->cluster().get();
        if (!geom->contains(id))
        {
          fwLog(fwlog::kWarning) << "failed get geometry of SiPixelCluster with detid: " << id << std::endl;
          continue;
        }

        float localPoint[3] = {
            fireworks::pixelLocalX((*itc).minPixelRow(), pars), fireworks::pixelLocalY((*itc).minPixelCol(), pars), 0.0};

        float globalPoint[3];
        geom->localToGlobal(id, localPoint, globalPoint);

        pointSet->SetNextPoint(globalPoint[0], globalPoint[1], globalPoint[2]);
        line->SetNextPoint(globalPoint[0], globalPoint[1], globalPoint[2]);
      }

      else
      {
        const SiStripCluster *cluster = fireworks::extractClusterFromTrackingRecHit(&hit);

        if (cluster)
        {
          short firststrip = cluster->firstStrip();
          float localTop[3] = {0.0, 0.0, 0.0};
          float localBottom[3] = {0.0, 0.0, 0.0};

          fireworks::localSiStrip(firststrip, localTop, localBottom, pars, id);

          float globalTop[3];
          float globalBottom[3];
          geom->localToGlobal(id, localTop, globalTop, localBottom, globalBottom);

          lineSet->AddLine(globalTop[0], globalTop[1], globalTop[2], globalBottom[0], globalBottom[1], globalBottom[2]);
        }
      }
    }

    SetupAddElement(pointSet, iItemHolder);
    SetupAddElement(line, iItemHolder);
    SetupAddElement(lineSet, iItemHolder);
  }
};

REGISTER_FW2PROXYBUILDER(FWTrajectorySeedProxyBuilder,
                        TrajectorySeed,
                        "TrajectorySeeds");
