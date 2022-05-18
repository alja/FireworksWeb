#include "ROOT/REveDataSimpleProxyBuilderTemplate.hxx"
#include "ROOT/REveStraightLineSet.hxx"
#include "ROOT/REveGeoShape.hxx"

#include "TGeoArb8.h"

#include "FireworksWeb/Core/interface/FWProxyBuilderFactory.h"
#include "FireworksWeb/Core/interface/FWGeometry.h"
#include "FireworksWeb/Core/interface/fwLog.h"
#include "FireworksWeb/Core/interface/Context.h"
#include "DataFormats/GEMRecHit/interface/GEMSegmentCollection.h"

using namespace ROOT::Experimental;

class FWGEMSegmentProxyBuilder : public REveDataSimpleProxyBuilderTemplate<GEMSegment>
{
public:
   REGISTER_FWPB_METHODS();
   virtual bool HaveSingleProduct() const {return false;}

   using REveDataSimpleProxyBuilderTemplate<GEMSegment>::BuildItemViewType;
   virtual void BuildItemViewType(const GEMSegment& iData, int /*idx*/,  REveElement*iItemHolder, const std::string& type , const REveViewContext*) override
   {
      const FWGeometry *geom = fireworks::Context::getInstance()->getGeom();
      unsigned int rawid = iData.gemDetId().rawId();

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
         if (TGeoTrap *box = dynamic_cast<TGeoTrap *>(shape->GetShape())) // Trapezoidal
         {
            shape->SetMainTransparency(75);
            shape->SetMainColor(Collection()->GetMainColor());
            SetupAddElement(shape, iItemHolder);
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
            if (type == "RPhi" && std::abs(dir.x()) < 0.1)
            {
               segmentSet->AddMarker(globalSegmentInnerPoint[0], globalSegmentInnerPoint[1], globalSegmentInnerPoint[2]);
            }
            else
               segmentSet->AddLine(globalSegmentInnerPoint[0],
                                   globalSegmentInnerPoint[1],
                                   globalSegmentInnerPoint[2],
                                   globalSegmentOuterPoint[0],
                                   globalSegmentOuterPoint[1],
                                   globalSegmentOuterPoint[2]);
         }
      }
   }
};

REGISTER_FW2PROXYBUILDER(FWGEMSegmentProxyBuilder, GEMSegment, "GEM Segment");
