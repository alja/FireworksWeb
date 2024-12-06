/*
 *  FWPSimHitProxyBuilder.cc
 *  FWorks
 *
 *  Created by Ianna Osborne on 9/9/10.
 *
 */

#include "FireworksWeb/Core/interface/FWWebEventItem.h"
#include "FireworksWeb/Core/interface/FWGeometry.h"
#include "FireworksWeb/Core/interface/fwLog.h"
#include "FireworksWeb/Core/interface/FWProxyBuilderFactory.h"

#include "SimDataFormats/TrackingHit/interface/PSimHit.h"
#include <DataFormats/MuonDetId/interface/DTWireId.h>


#include "ROOT/REveDataSimpleProxyBuilderTemplate.hxx"
#include "ROOT/REvePointSet.hxx"


using namespace ROOT::Experimental;
class FWPSimHitProxyBuilder : public REveDataSimpleProxyBuilderTemplate<PSimHit> {
public:
  FWPSimHitProxyBuilder(void) {}
  ~FWPSimHitProxyBuilder(void) override {}

   REGISTER_FWPB_METHODS();
   using REveDataSimpleProxyBuilderTemplate<PSimHit>::HaveSingleProduct;
   bool HaveSingleProduct() const override { return false; }


  // Disable default copy constructor
  FWPSimHitProxyBuilder(const FWPSimHitProxyBuilder&) = delete;
  // Disable default assignment operator
  const FWPSimHitProxyBuilder& operator=(const FWPSimHitProxyBuilder&) = delete;

  using REveDataSimpleProxyBuilderTemplate<PSimHit>::BuildItemViewType;
  void BuildItemViewType(const PSimHit& iData, int iIndex, REveElement* iItemHolder, const std::string& viewType, const REveViewContext*) override
  {
    REvePointSet* pointSet = new REvePointSet();
    SetupAddElement(pointSet, iItemHolder, true);
    const FWGeometry *geom = fireworks::Context::getInstance()->getGeom();
    unsigned int rawid = iData.detUnitId();
    if (!geom->contains(rawid)) {
      fwLog(fwlog::kError) << "failed to get geometry of detid: " << rawid << std::endl;
      return;
    }

    float local[3] = {iData.localPosition().x(), iData.localPosition().y(), iData.localPosition().z()};
    float global[3];

    // Specialized for DT simhits
    DetId id(rawid);
    if (id.det() == DetId::Muon && id.subdetId() == 1) {
      DTWireId wId(rawid);
      rawid = wId.layerId().rawId();  // DT simhits are in the RF of the DTLayer, but their ID is the id of the wire!
      if (abs(iData.particleType()) != 13) {
        pointSet->SetMarkerStyle(4);  // Draw non-muon simhits (e.g. delta rays) with a different marker
      }
      if (viewType == "kRhoZ") {  //
        // In RhoZ view, draw hits at the middle of the layer in the global Z coordinate,
        // otherwise they won't align with 1D rechits, for which only one coordinate is known.
        if (wId.superLayer() == 2) {
          local[1] = 0;
        } else {
          local[0] = 0;
        }
      }
    }

    geom->localToGlobal(rawid, local, global);
    pointSet->SetNextPoint(global[0], global[1], global[2]);
  }
};

REGISTER_FW2PROXYBUILDER(FWPSimHitProxyBuilder, PSimHit, "PSimHits");