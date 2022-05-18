#include "ROOT/REveDataSimpleProxyBuilderTemplate.hxx"
#include "ROOT/REveStraightLineSet.hxx"
#include "ROOT/REveGeoShape.hxx"
#include "ROOT/REvePointSet.hxx"

#include "TGeoArb8.h"

#include "FireworksWeb/Core/interface/FWProxyBuilderFactory.h"
#include "FireworksWeb/Core/interface/FWGeometry.h"
#include "FireworksWeb/Core/interface/fwLog.h"
#include "FireworksWeb/Core/interface/Context.h"

#include "DataFormats/MuonDetId/interface/DTChamberId.h"
#include "DataFormats/DTRecHit/interface/DTRecSegment4DCollection.h"

using namespace ROOT::Experimental;

class FWDTSegmentProxyBuilder : public REveDataSimpleProxyBuilderTemplate<DTRecSegment4D>
{
public:
   REGISTER_FWPB_METHODS();
   virtual bool HaveSingleProduct() const {return false;}

   using REveDataSimpleProxyBuilderTemplate<DTRecSegment4D>::BuildItemViewType;
   virtual void BuildItemViewType(const DTRecSegment4D& iData, int /*idx*/,  REveElement*iItemHolder, const std::string& type , const REveViewContext*) override
   {
      const FWGeometry *geom = fireworks::Context::getInstance()->getGeom();
      unsigned int rawid = iData.chamberId().rawId();

      if (!geom->contains(rawid))
      {
         fwLog(fwlog::kError) << "failed to get geometry of CSC chamber with rawid: "
                              << rawid << std::endl;
         return;
      }

      REveStraightLineSet *segmentSet = new REveStraightLineSet();
      // FIXME: This should be set elsewhere.
      segmentSet->SetLineWidth(4);
      SetupAddElement(segmentSet, iItemHolder);

      REveGeoShape *shape = geom->getEveShape(rawid);
      if (shape)
      {
         if (TGeoBBox *box = dynamic_cast<TGeoBBox *>(shape->GetShape()))
         {
            LocalPoint pos = iData.localPosition();
            LocalVector dir = iData.localDirection();
            LocalVector unit = dir.unit();

            double localPosition[3] = {pos.x(), pos.y(), pos.z()};
            double localDirectionIn[3] = {dir.x(), dir.y(), dir.z()};
            double localDirectionOut[3] = {-dir.x(), -dir.y(), -dir.z()};

            // In RhoZ view, draw segments at the middle of the chamber, otherwise they won't align with 1D rechits,
            // for which only one coordinate is known.
            if (type == "RhoZ")
            {
               localPosition[0] = 0;
               localDirectionIn[0] = 0;
               localDirectionOut[0] = 0;
            }

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
            pointSet->SetMarkerSize(1.5);
            SetupAddElement(pointSet, iItemHolder);

            std::vector<DTRecHit1D> recHits;
            const DTChamberRecSegment2D *phiSeg = iData.phiSegment();
            const DTSLRecSegment2D *zSeg = iData.zSegment();
            if (phiSeg)
            {
               std::vector<DTRecHit1D> phiRecHits = phiSeg->specificRecHits();
               copy(phiRecHits.begin(), phiRecHits.end(), back_inserter(recHits));
            }
            if (zSeg)
            {
               std::vector<DTRecHit1D> zRecHits = zSeg->specificRecHits();
               copy(zRecHits.begin(), zRecHits.end(), back_inserter(recHits));
            }

            for (std::vector<DTRecHit1D>::const_iterator rh = recHits.begin(); rh != recHits.end(); ++rh)
            {
               DTLayerId layerId = (*rh).wireId().layerId();
               LocalPoint hpos = (*rh).localPosition();
               float hitLocalPos[3] = {hpos.x(), hpos.y(), hpos.z()};
               if (type == "RhoZ")
               {
                  // In RhoZ view, draw hits at the middle of the layer in the global Z coordinate,
                  // otherwise they won't align with 1D rechits, for which only one coordinate is known.
                  if (layerId.superLayer() == 2)
                  {
                     hitLocalPos[1] = 0;
                  }
                  else
                  {
                     hitLocalPos[0] = 0;
                  }
               }
               float hitGlobalPoint[3];
               geom->localToGlobal(layerId, hitLocalPos, hitGlobalPoint);
               pointSet->SetNextPoint(hitGlobalPoint[0], hitGlobalPoint[1], hitGlobalPoint[2]);
            }
         }
      }
   }
};

REGISTER_FW2PROXYBUILDER(FWDTSegmentProxyBuilder, DTRecSegment4D, "DT-segment");
