
#include "TClass.h"

#include "FireworksWeb/Core/interface/FWItemRandomAccessor.h"
#include "FireworksWeb/Core/interface/FWItemSingleAccessor.h"
#include "FireworksWeb/Core/interface/FWProxyBuilderFactory.h"



#include "SimDataFormats/Associations/interface/LayerClusterToCaloParticleAssociatorBaseImpl.h"
class RecoToSimSingleAccessor : public FWItemSingleAccessor {
public:
  RecoToSimSingleAccessor(const TClass* x) : FWItemSingleAccessor(x) {}
  ~RecoToSimSingleAccessor() override{};
  REGISTER_FWITEMACCESSOR_METHODS();
};
/*
REGISTER_TEMPLATE_FWITEMACCESSOR(FWItemDetSetAccessor<edm::DetSetVector<SiStripDigi> >,
                                 edm::DetSetVector<SiStripDigi>,
                                 "SiStripDigiCollectionAccessor");
REGISTER_TEMPLATE_FWITEMACCESSOR(FWItemDetSetAccessor<edm::DetSetVector<PixelDigi> >,
                                 edm::DetSetVector<PixelDigi>,
                                 "SiPixelDigiCollectionAccessor");
REGISTER_TEMPLATE_FWITEMACCESSOR(FWItemNewDetSetAccessor<edmNew::DetSetVector<SiStripCluster> >,
                                 edmNew::DetSetVector<SiStripCluster>,
                                 "SiStripClusterCollectionNewAccessor");
REGISTER_TEMPLATE_FWITEMACCESSOR(FWItemNewDetSetAccessor<edmNew::DetSetVector<SiPixelCluster> >,
                                 edmNew::DetSetVector<SiPixelCluster>,
                                 "SiPixelClusterCollectionNewAccessor");
REGISTER_TEMPLATE_FWITEMACCESSOR(FWItemNewDetSetAccessor<edmNew::DetSetVector<Phase2TrackerCluster1D> >,
                                 edmNew::DetSetVector<Phase2TrackerCluster1D>,
                                 "Phase2TrackerCluster1DCollectionNewAccessor");
*/
REGISTER_FWITEMACCESSOR(RecoToSimSingleAccessor, hgcal::RecoToSimCollection, "RecoToSimAccessor");

