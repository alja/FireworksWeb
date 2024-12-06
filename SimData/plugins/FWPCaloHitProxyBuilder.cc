/*
 *  FWPCaloHitProxyBuilder.cc
 *  FWorks
 *
 *  Created by Ianna Osborne on 9/9/10.
 *
 */

#include "FireworksWeb/Core/interface/FWDigitSetProxyBuilder.h"
#include "FireworksWeb/Core/interface/FWGeometry.h"
#include "FireworksWeb/Core/interface/Context.h"
#include "FireworksWeb/Core/interface/BuilderUtils.h"
#include "DataFormats/EcalDetId/interface/EcalSubdetector.h"
#include "SimDataFormats/CaloHit/interface/PCaloHit.h"
#include "SimDataFormats/CaloHit/interface/PCaloHitContainer.h"
#include "FireworksWeb/Core/interface/FWProxyBuilderFactory.h"

using namespace ROOT::Experimental;

// AMT: just like original implementation, this is missing energy scaling

class FWPCaloHitProxyBuilder : public FWDigitSetProxyBuilder {
public:
   REGISTER_FWPB_METHODS();

 using REveDataProxyBuilderBase::BuildProduct;
void BuildProduct(const ROOT::Experimental::REveDataCollection* collection, ROOT::Experimental::REveElement* product, const ROOT::Experimental::REveViewContext*)
{
    size_t size = Collection()->GetNItems();

    if (!product->HasChildren())
       m_boxSet = addBoxSetToProduct(product);

    m_boxSet->Reset(REveBoxSet::kBT_FreeBox, true, size);
    for (int index = 0; index < static_cast<int>(size); ++index)
    {
        const PCaloHit *hit = (const PCaloHit*)Collection()->GetDataPtr(index);
       const float* corners = fireworks::Context::getInstance()->getGeom()->getCorners(hit->id());
       std::vector<float> scaledCorners(24);
       if (corners)
           fireworks::energyTower3DCorners(corners, hit->energy() * 10, scaledCorners);

    addBox(m_boxSet, &scaledCorners[0], Collection()->GetDataItem(index));
  }
    
}
};

REGISTER_FW2PROXYBUILDER_BASE(FWPCaloHitProxyBuilder, edm::PCaloHitContainer,  "PCaloHits");