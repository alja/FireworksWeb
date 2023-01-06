// -*- C++ -*-
//
// Package:     Muons
// Class  :     FWDTRecHitProxyBuilder
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

#include "DataFormats/MuonDetId/interface/DTChamberId.h"
#include "DataFormats/DTRecHit/interface/DTRecHitCollection.h"

using namespace ROOT::Experimental;
using namespace DTEnums;

class FWDTRecHitProxyBuilder : public REveDataSimpleProxyBuilderTemplate<DTRecHit1DPair>
{
public:
   REGISTER_FWPB_METHODS();
   virtual bool HaveSingleProduct() const {return false;}

   using REveDataSimpleProxyBuilderTemplate<DTRecHit1DPair>::BuildItemViewType; 
   virtual void BuildItemViewType(const DTRecHit1DPair& iData, int /*idx*/,  REveElement* oItemHolder, const std::string& type , const REveViewContext*) override
  
    {
      const FWGeometry *geom = fireworks::Context::getInstance()->getGeom();

      const DTLayerId& layerId = iData.wireId().layerId();
      int superLayer = layerId.superlayerId().superLayer();

      if (!geom->contains(layerId)) {
         fwLog(fwlog::kError) << "failed get geometry of DT layer with detid: " << layerId << std::endl;
         return;
      }

      const DTRecHit1D* leftRecHit = iData.componentRecHit(Left);
      const DTRecHit1D* rightRecHit = iData.componentRecHit(Right);
      float lLocalPos[3] = {leftRecHit->localPosition().x(), 0.0, 0.0};
      float rLocalPos[3] = {rightRecHit->localPosition().x(), 0.0, 0.0};

      if ((type == "RPhi" && superLayer != 2) ||
         (type == "RhoZ" && superLayer == 2) || type == "3D") {
         float leftGlobalPoint[3];
         float rightGlobalPoint[3];

         REveStraightLineSet* recHitSet = new REveStraightLineSet;
         SetupAddElement(recHitSet, oItemHolder);

         REvePointSet* pointSet = new REvePointSet;
         pointSet->SetMarkerSize(1.5*fireworks::Context::s_markerScale);
         SetupAddElement(pointSet, oItemHolder);


         geom->localToGlobal(layerId, lLocalPos, leftGlobalPoint, rLocalPos, rightGlobalPoint);

         pointSet->SetNextPoint(leftGlobalPoint[0], leftGlobalPoint[1], leftGlobalPoint[2]);
         pointSet->SetNextPoint(rightGlobalPoint[0], rightGlobalPoint[1], rightGlobalPoint[2]);

         recHitSet->AddLine(leftGlobalPoint[0],
                           leftGlobalPoint[1],
                           leftGlobalPoint[2],
                           rightGlobalPoint[0],
                           rightGlobalPoint[1],
                           rightGlobalPoint[2]);
      }
    }
};

REGISTER_FW2PROXYBUILDER(FWDTRecHitProxyBuilder, DTRecHit1DPair, "DT RecHits");
