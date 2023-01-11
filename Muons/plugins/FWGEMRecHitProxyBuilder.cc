// -*- C++ -*-
//
// Package:     Muons
// Class  :     FWGEMRecHitProxyBuilder
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

#include "DataFormats/GEMRecHit/interface/GEMRecHitCollection.h"

using namespace ROOT::Experimental;

class FWGEMRecHitProxyBuilder : public REveDataSimpleProxyBuilderTemplate<GEMRecHit>
{
public:
   REGISTER_FWPB_METHODS();
   virtual bool HaveSingleProduct() const {return false;}

   using REveDataSimpleProxyBuilderTemplate<GEMRecHit>::BuildItemViewType; 
   virtual void BuildItemViewType(const GEMRecHit& iData, int /*idx*/,  REveElement* oItemHolder, const std::string& type , const REveViewContext*) override
  
    {
      const FWGeometry *geom = fireworks::Context::getInstance()->getGeom();
      GEMDetId gemId = iData.gemId();
      unsigned int rawid = gemId.rawId();

      if (!geom->contains(rawid)) {
         fwLog(fwlog::kError) << "failed to get geometry of GEM roll with detid: " << rawid << std::endl;
         return;
      }

      REveStraightLineSet* recHitSet = new REveStraightLineSet;
      recHitSet->SetLineWidth(3);

      if (type == "3D") {
         REveGeoShape* shape = geom->getEveShape(rawid);
         shape->SetMainTransparency(75);
         SetupAddElement(shape, oItemHolder);
      }

      float localX = iData.localPosition().x();
      float localY = iData.localPosition().y();
      float localZ = iData.localPosition().z();

      float localXerr = sqrt(iData.localPositionError().xx());
      float localYerr = sqrt(iData.localPositionError().yy());

      float localU1[3] = {localX - localXerr, localY, localZ};

      float localU2[3] = {localX + localXerr, localY, localZ};

      float localV1[3] = {localX, localY - localYerr, localZ};

      float localV2[3] = {localX, localY + localYerr, localZ};

      float globalU1[3];
      float globalU2[3];
      float globalV1[3];
      float globalV2[3];

      FWGeometry::IdToInfoItr det = geom->find(rawid);

      geom->localToGlobal(*det, localU1, globalU1);
      geom->localToGlobal(*det, localU2, globalU2);
      geom->localToGlobal(*det, localV1, globalV1);
      geom->localToGlobal(*det, localV2, globalV2);

      recHitSet->AddLine(globalU1[0], globalU1[1], globalU1[2], globalU2[0], globalU2[1], globalU2[2]);

      recHitSet->AddLine(globalV1[0], globalV1[1], globalV1[2], globalV2[0], globalV2[1], globalV2[2]);

      SetupAddElement(recHitSet, oItemHolder);
    }
};

REGISTER_FW2PROXYBUILDER(FWGEMRecHitProxyBuilder, GEMRecHit, "GEM RecHits");
