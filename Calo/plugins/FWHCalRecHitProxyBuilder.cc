

#include "FireworksWeb/Calo/interface/FWCaloRecHitDigitSetProxyBuilder.h"
#include "DataFormats/HcalRecHit/interface/HcalRecHitCollections.h"


#include "FireworksWeb/Core/interface/FWProxyBuilderFactory.h"

class FWHBHERecHitProxyBuilder : public FWCaloRecHitDigitSetProxyBuilder {
public:
   REGISTER_FWPB_METHODS();
};

REGISTER_FW2PROXYBUILDER_BASE(FWHBHERecHitProxyBuilder, HBHERecHitCollection, "HBHE RecHit");


class FWHFRecHitProxyBuilder : public FWCaloRecHitDigitSetProxyBuilder {
public:
   REGISTER_FWPB_METHODS();
};

REGISTER_FW2PROXYBUILDER_BASE(FWHFRecHitProxyBuilder, HFRecHitCollection, "HF RecHit");

class FWHORecHitProxyBuilder : public FWCaloRecHitDigitSetProxyBuilder {
public:
   REGISTER_FWPB_METHODS();
};

REGISTER_FW2PROXYBUILDER_BASE(FWHORecHitProxyBuilder, HORecHitCollection, "HO RecHit");
