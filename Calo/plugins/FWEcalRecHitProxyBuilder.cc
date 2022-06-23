
#include "DataFormats/EcalDetId/interface/EcalSubdetector.h"
#include "FireworksWeb/Calo/interface/FWCaloRecHitDigitSetProxyBuilder.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"


#include "FireworksWeb/Core/interface/FWProxyBuilderFactory.h"

class FWEcalRecHitProxyBuilder : public FWCaloRecHitDigitSetProxyBuilder {
public:
  FWEcalRecHitProxyBuilder() {}
  ~FWEcalRecHitProxyBuilder() override {}

  void viewContextBoxScale(
      const float *corners, float scale, bool plotEt, std::vector<float> &scaledCorners, const CaloRecHit *hit) override
  {
      invertBox((EcalSubdetector(hit->detid().subdetId()) == EcalPreshower) && (corners[2] < 0));
      FWCaloRecHitDigitSetProxyBuilder::viewContextBoxScale(corners, scale, plotEt, scaledCorners, hit);
  }

   REGISTER_FWPB_METHODS();

private:
  FWEcalRecHitProxyBuilder(const FWEcalRecHitProxyBuilder&) = delete;
  const FWEcalRecHitProxyBuilder& operator=(const FWEcalRecHitProxyBuilder&) = delete;
};

REGISTER_FW2PROXYBUILDER_BASE(FWEcalRecHitProxyBuilder, EcalRecHitCollection, "Ecal RecHit");
