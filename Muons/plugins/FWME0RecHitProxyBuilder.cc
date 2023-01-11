// -*- C++ -*-
//
// Package:     Muons
// Class  :     FWME0RecHitProxyBuilder
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

#include "DataFormats/GEMRecHit/interface/ME0RecHitCollection.h"

using namespace ROOT::Experimental;

class FWME0RecHitProxyBuilder : public REveDataSimpleProxyBuilderTemplate<ME0RecHit>
{
public:
   REGISTER_FWPB_METHODS();
   virtual bool HaveSingleProduct() const {return false;}

   using REveDataSimpleProxyBuilderTemplate<ME0RecHit>::BuildItemViewType; 
   virtual void BuildItemViewType(const ME0RecHit& iData, int /*idx*/,  REveElement* oItemHolder, const std::string& type , const REveViewContext*) override
   {
      ME0DetId me0Id = iData.me0Id();
      unsigned int rawid = me0Id.rawId();
      const FWGeometry *geom = fireworks::Context::getInstance()->getGeom();


      if (!geom->contains(rawid)) {
         fwLog(fwlog::kError) << "failed to get geometry of ME0 roll with detid: " << rawid << std::endl;
         return;
      }

      REveStraightLineSet* recHitSet = new REveStraightLineSet;
      recHitSet->SetLineWidth(3);

      if (type == "3D") {
         REveGeoShape* shape = geom->getEveShape(rawid);
         shape->SetMainTransparency(75);
         SetupAddElement(shape, oItemHolder);
         recHitSet->AddElement(shape);
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

REGISTER_FW2PROXYBUILDER(FWME0RecHitProxyBuilder, ME0RecHit, "ME0 RecHits");
