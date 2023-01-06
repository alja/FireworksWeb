// -*- C++ -*-
//
// Package:     Muons
// Class  :     FWME0SegmentProxyBuilder
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
#include "DataFormats/GEMRecHit/interface/ME0SegmentCollection.h"

using namespace ROOT::Experimental;

// !! AMT this is not tested !!!!

class FWME0SegmentProxyBuilder : public REveDataSimpleProxyBuilderTemplate<ME0Segment>
{
public:
   REGISTER_FWPB_METHODS();
   using REveDataSimpleProxyBuilderTemplate<ME0Segment>::BuildItem;
   virtual void BuildItem(const ME0Segment& iData, int /*idx*/, ROOT::Experimental::REveElement* iItemHolder, const ROOT::Experimental::REveViewContext* vc) override
   {
      const FWGeometry *geom = fireworks::Context::getInstance()->getGeom();
      unsigned int rawid = iData.me0DetId().rawId();

      if( ! geom->contains( rawid ))
      {
         fwLog(fwlog::kError) << "failed to get geometry of CSC chamber with rawid: "
                              << rawid << std::endl;
         return;
      }

      REveStraightLineSet* segmentSet = new REveStraightLineSet();
      // FIXME: This should be set elsewhere.
      segmentSet->SetLineWidth( 4 );
      SetupAddElement( segmentSet, iItemHolder );

      TGeoShape *shape = geom->getShape(rawid);
      if (shape)
      {
         if (TGeoBBox *box = dynamic_cast<TGeoBBox *>(shape))
         {
            LocalPoint pos = iData.localPosition();
            LocalVector dir = iData.localDirection();
            LocalVector unit = dir.unit();

            double localPosition[3] = {pos.x(), pos.y(), pos.z()};
            double localDirectionIn[3] = {dir.x(), dir.y(), dir.z()};
            double localDirectionOut[3] = {-dir.x(), -dir.y(), -dir.z()};

            Double_t distIn = box->DistFromInside(localPosition, localDirectionIn);
            Double_t distOut = box->DistFromInside(localPosition, localDirectionOut);
            LocalVector vIn = unit * distIn;
            LocalVector vOut = -unit * distOut;
            float localSegmentInnerPoint[3] = {static_cast<float>(localPosition[0] + vIn.x()),
                                               static_cast<float>(localPosition[1] + vIn.y()),
                                               static_cast<float>(localPosition[2] + vIn.z())};

            float localSegmentOuterPoint[3] = {static_cast<float>(localPosition[0] + vOut.x()),
                                               static_cast<float>(localPosition[1] + vOut.y()),
                                               static_cast<float>(localPosition[2] + vOut.z())};

            float globalSegmentInnerPoint[3];
            float globalSegmentOuterPoint[3];

            geom->localToGlobal(
                rawid, localSegmentInnerPoint, globalSegmentInnerPoint, localSegmentOuterPoint, globalSegmentOuterPoint);

            segmentSet->AddLine(globalSegmentInnerPoint[0],
                                globalSegmentInnerPoint[1],
                                globalSegmentInnerPoint[2],
                                globalSegmentOuterPoint[0],
                                globalSegmentOuterPoint[1],
                                globalSegmentOuterPoint[2]);

            // Draw hits included in the segment
            REvePointSet *pointSet = new REvePointSet;
            // FIXME: This should be set elsewhere.
            pointSet->SetMarkerSize(fireworks::Context::s_markerScale);
            SetupAddElement(pointSet, iItemHolder);
            auto recHits = iData.specificRecHits();
            for (auto rh = recHits.begin(); rh != recHits.end(); rh++)
            {
               auto me0id = rh->me0Id();
               LocalPoint hpos = rh->localPosition();
               float hitLocalPos[3] = {hpos.x(), hpos.y(), hpos.z()};
               float hitGlobalPoint[3];
               geom->localToGlobal(me0id, hitLocalPos, hitGlobalPoint);
               pointSet->SetNextPoint(hitGlobalPoint[0], hitGlobalPoint[1], hitGlobalPoint[2]);
            }
         }
      }
   }
};

REGISTER_FW2PROXYBUILDER(FWME0SegmentProxyBuilder, ME0Segment, "MEO-segments");
