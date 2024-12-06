
#include "FireworksWeb/Core/interface/Context.h"
#include "FireworksWeb/Core/interface/FWGeometry.h"
#include "FireworksWeb/Core/interface/FWProxyBuilderFactory.h"

#include "SimDataFormats/CaloAnalysis/interface/CaloParticle.h"
#include "SimDataFormats/CaloAnalysis/interface/CaloParticleFwd.h"
#include "SimDataFormats/CaloAnalysis/interface/SimCluster.h"

#include "ROOT/REveBoxSet.hxx"
#include "ROOT/REveDataSimpleProxyBuilderTemplate.hxx"

using namespace ROOT::Experimental;

class FWTrackProxyBuilder : public REveDataSimpleProxyBuilderTemplate<CaloParticle>
{
public:
   REGISTER_FWPB_METHODS();

   using REveDataSimpleProxyBuilderTemplate<CaloParticle>::BuildItem;
   virtual void BuildItem(const CaloParticle& iData, int /*idx*/, REveElement* iItemHolder, const REveViewContext* vc) override
   {
      REveBoxSet *boxset = new REveBoxSet();
      boxset->UseSingleColor();
      boxset->Reset(REveBoxSet::kBT_FreeBox, true, 64);
      boxset->SetAntiFlick(true);

      auto geom = fireworks::Context::getInstance()->getGeom();
      for (const auto &c : iData.simClusters()) {
        for (const auto &it : (*c).hits_and_fractions()) {
          if (DetId(it.first).det() != DetId::Detector::Ecal) {
            std::cerr << "this proxy should be used only for ECAL";
            return;
          }

          const float *corners = geom->getCorners(it.first);
          if (corners == nullptr)
            continue;

          boxset->AddBox(corners);
        }
      }

      boxset->RefitPlex();
      boxset->CSCApplyMainColorToMatchingChildren();
      boxset->CSCApplyMainTransparencyToMatchingChildren();
      SetupAddElement(boxset, iItemHolder);
   }
};


REGISTER_FW2PROXYBUILDER(FWTrackProxyBuilder, CaloParticle, "ECaloParticle");

/*
class FWECaloParticleProxyBuilder : public FWSimpleProxyBuilderTemplate<CaloParticle> {
public:
  FWECaloParticleProxyBuilder(void) {}
  ~FWECaloParticleProxyBuilder(void) override {}

  REGISTER_PROXYBUILDER_METHODS();

  // Disable default copy constructor
  FWECaloParticleProxyBuilder(const FWECaloParticleProxyBuilder &) = delete;
  // Disable default assignment operator
  const FWECaloParticleProxyBuilder &operator=(const FWECaloParticleProxyBuilder &) = delete;

private:
  void build(const CaloParticle &iData, unsigned int iIndex, TEveElement &oItemHolder, const FWViewContext *) override;
};

void FWECaloParticleProxyBuilder::build(const CaloParticle &iData,
                                        unsigned int iIndex,
                                        TEveElement &oItemHolder,
                                        const FWViewContext *) {
  TEveBoxSet *boxset = new TEveBoxSet();
  boxset->UseSingleColor();
  boxset->SetPickable(true);
  boxset->Reset(TEveBoxSet::kBT_FreeBox, true, 64);
  boxset->SetAntiFlick(true);

  for (const auto &c : iData.simClusters()) {
    for (const auto &it : (*c).hits_and_fractions()) {
      if (DetId(it.first).det() != DetId::Detector::Ecal) {
        std::cerr << "this proxy should be used only for ECAL";
        return;
      }

      const float *corners = item()->getGeom()->getCorners(it.first);
      if (corners == nullptr)
        continue;

      boxset->AddBox(corners);
    }
  }

  boxset->RefitPlex();
  boxset->CSCTakeAnyParentAsMaster();
  boxset->CSCApplyMainColorToMatchingChildren();
  boxset->CSCApplyMainTransparencyToMatchingChildren();
  boxset->SetMainColor(item()->modelInfo(iIndex).displayProperties().color());
  boxset->SetMainTransparency(item()->defaultDisplayProperties().transparency());
  oItemHolder.AddElement(boxset);
}

REGISTER_FWPROXYBUILDER(FWECaloParticleProxyBuilder,
                        CaloParticle,
                        "ECaloParticle",
                        FWViewType::kAll3DBits | FWViewType::kAllRPZBits);
*/