// -*- C++ -*-
//
// Package:     Muons
// Class  :     FWCSCRecHitProxyBuilder
//
// Implementation:
//     <Notes on implementation>
//
// Original Author:
//         Created:  Sun Jan  6 23:57:00 EST 2008
//

#include "ROOT/REveDataSimpleProxyBuilderTemplate.hxx"
#include "ROOT/REveStraightLineSet.hxx"
#include "ROOT/REvePointSet.hxx"
#include "ROOT/REveGeoShape.hxx"

#include "TGeoArb8.h"

#include "FireworksWeb/Core/interface/FWProxyBuilderFactory.h"
#include "FireworksWeb/Core/interface/FWGeometry.h"
#include "FireworksWeb/Core/interface/fwLog.h"
#include "FireworksWeb/Core/interface/Context.h"

#include "DataFormats/CSCRecHit/interface/CSCRecHit2DCollection.h"

using namespace ROOT::Experimental;

class FWCSCRecHitProxyBuilder : public REveDataSimpleProxyBuilderTemplate<CSCRecHit2D>
{
public:
   REGISTER_FWPB_METHODS();
   using REveDataSimpleProxyBuilderTemplate<CSCRecHit2D>::BuildItem;
   virtual void BuildItem(const CSCRecHit2D &iData, int /*idx*/, ROOT::Experimental::REveElement *iItemHolder, const ROOT::Experimental::REveViewContext *vc) override
   {
      const FWGeometry *geom = fireworks::Context::getInstance()->getGeom();

      unsigned int rawid = iData.cscDetId().rawId();

      if (!geom->contains(rawid))
      {
         fwLog(fwlog::kError) << "failed to get geometry of CSC layer with detid: " << rawid << std::endl;
         return;
      }
      FWGeometry::IdToInfoItr det = geom->find(rawid);

      REveStraightLineSet *recHitSet = new REveStraightLineSet;
      SetupAddElement(recHitSet, iItemHolder);

      REvePointSet *pointSet = new REvePointSet;
      pointSet->SetMarkerSize(1*fireworks::Context::s_markerScale);
      SetupAddElement(pointSet, iItemHolder);

      float localPositionX = iData.localPosition().x();
      float localPositionY = iData.localPosition().y();

      float localPositionXX = sqrt(iData.localPositionError().xx());
      float localPositionYY = sqrt(iData.localPositionError().yy());

      float localU1Point[3] = {localPositionX - localPositionXX, localPositionY, 0.0};

      float localU2Point[3] = {localPositionX + localPositionXX, localPositionY, 0.0};

      float localV1Point[3] = {localPositionX, localPositionY - localPositionYY, 0.0};

      float localV2Point[3] = {localPositionX, localPositionY + localPositionYY, 0.0};

      float globalU1Point[3];
      float globalU2Point[3];
      float globalV1Point[3];
      float globalV2Point[3];

      geom->localToGlobal(*det, localU1Point, globalU1Point);
      geom->localToGlobal(*det, localU2Point, globalU2Point);
      geom->localToGlobal(*det, localV1Point, globalV1Point);
      geom->localToGlobal(*det, localV2Point, globalV2Point);

      pointSet->SetNextPoint(globalU1Point[0], globalU1Point[1], globalU1Point[2]);
      pointSet->SetNextPoint(globalU2Point[0], globalU2Point[1], globalU2Point[2]);
      pointSet->SetNextPoint(globalV1Point[0], globalV1Point[1], globalV1Point[2]);
      pointSet->SetNextPoint(globalV2Point[0], globalV2Point[1], globalV2Point[2]);

      recHitSet->AddLine(
          globalU1Point[0], globalU1Point[1], globalU1Point[2], globalU2Point[0], globalU2Point[1], globalU2Point[2]);

      recHitSet->AddLine(
          globalV1Point[0], globalV1Point[1], globalV1Point[2], globalV2Point[0], globalV2Point[1], globalV2Point[2]);

      // printf("add line (%f, %f, %f) , %f, %f, %f \n", globalU1Point[0], globalU1Point[1], globalU1Point[2], globalU2Point[0], globalU2Point[1], globalU2Point[2]);    
   }
};

REGISTER_FW2PROXYBUILDER(FWCSCRecHitProxyBuilder, CSCRecHit2D, "CSC RecHits");
