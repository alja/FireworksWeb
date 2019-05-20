// -*- C++ -*-
//
// Package:     Muons
// Class  :     FWCSCSegmentProxyBuilder
//
// Implementation:
//     <Notes on implementation>
//
// Original Author:
//         Created:  Sun Jan  6 23:57:00 EST 2008
//

//#include "REveGeoNode.h"
#include "ROOT/REveStraightLineSet.hxx"
#include "TGeoArb8.h"

#include "Fireworks2/Core/interface/FWGeometry.h"
#include "Fireworks2/Core/interface/fwLog.h"

#include "DataFormats/CSCRecHit/interface/CSCSegmentCollection.h"

using namespace ROOT::Experimental;

class FWCSCSegmentProxyBuilder : public REveDataSimpleProxyBuilderTemplate<CSCSegment>
{   
   using REveDataSimpleProxyBuilderTemplate<CSCSegment>::Build;
   virtual void Build(const reco::Track& iData, REX::REveElement* iItemHolder, const REX::REveViewContext* vc)
   {
      const FWGeometry *geom = fireworks::Context::getInstance()->getGeom();
      unsigned int rawid = iData.cscDetId().rawId();
  
      if( ! geom->contains( rawid ))
      {
         fwLog(fwlog::kError) << "failed to get geometry of CSC chamber with rawid: " 
                              << rawid << std::endl;
         return;
      }
  
      REveStraightLineSet* segmentSet = new REveStraightLineSet();
      // FIXME: This should be set elsewhere.
      segmentSet->SetLineWidth( 3 );
      SetupAddElement( segmentSet, iItemHolder );

      REveGeoShape* shape = geom->getEveShape( rawid );
      if( TGeoTrap* trap = dynamic_cast<TGeoTrap*>( shape->GetShape())) // Trapezoidal
      {
         LocalPoint pos = iData.localPosition();
         LocalVector dir = iData.localDirection();   
         LocalVector unit = dir.unit();
    
         Double_t localPosition[3]     = {  pos.x(),  pos.y(),  pos.z() };
         Double_t localDirectionIn[3]  = {  dir.x(),  dir.y(),  dir.z() };
         Double_t localDirectionOut[3] = { -dir.x(), -dir.y(), -dir.z() };
  
         float distIn = trap->DistFromInside( localPosition, localDirectionIn );
         float distOut = trap->DistFromInside( localPosition, localDirectionOut );
         LocalVector vIn = unit * distIn;
         LocalVector vOut = -unit * distOut;
         float localSegmentInnerPoint[3] = { static_cast<float>(localPosition[0] + vIn.x()),
                                             static_cast<float>(localPosition[1] + vIn.y()),
                                             static_cast<float>(localPosition[2] + vIn.z()) 
         };
      
         float localSegmentOuterPoint[3] = { static_cast<float>(localPosition[0] + vOut.x()),
                                             static_cast<float>(localPosition[1] + vOut.y()),
                                             static_cast<float>(localPosition[2] + vOut.z()) 
         };

         float globalSegmentInnerPoint[3];
         float globalSegmentOuterPoint[3];
     
         geom->localToGlobal( rawid, localSegmentInnerPoint,  globalSegmentInnerPoint, localSegmentOuterPoint,  globalSegmentOuterPoint );

         segmentSet->AddLine( globalSegmentInnerPoint[0], globalSegmentInnerPoint[1], globalSegmentInnerPoint[2],
                              globalSegmentOuterPoint[0], globalSegmentOuterPoint[1], globalSegmentOuterPoint[2] );	
      }
   }
};
