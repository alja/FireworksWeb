#include "FireworksWeb/Core/interface/FWProxyBuilderFactory.h"

#include "ROOT/REvePointSet.hxx"
#include "ROOT/REveViewContext.hxx"
#include "ROOT/REveDataSimpleProxyBuilderTemplate.hxx"

#include "FireworksWeb/Core/interface/FWGeometry.h"
#include "FireworksWeb/Core/interface/fwLog.h"
#include "FireworksWeb/Core/interface/Context.h"

#include "DataFormats/CaloRecHit/interface/CaloClusterFwd.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/FWLite/interface/Event.h"
#include "DataFormats/FWLite/interface/Handle.h"

// #include  "SimDataFormats/Associations/interface/LayerClusterToCaloParticleAssociatorBaseImpl.h"

//#include "FWCore/Framework/interface/ESHandle.h"
//#include "FWCore/Framework/interface/Event.h"
//#include "FWCore/Framework/interface/Frameworkfwd.h"
// #include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/Common/interface/AssociationMap.h"
#include "SimDataFormats/CaloAnalysis/interface/CaloParticle.h"




#include "DataFormats/CaloRecHit/interface/CaloClusterFwd.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/FWLite/interface/Event.h"
#include "DataFormats/FWLite/interface/Handle.h"

#include "DataFormats/Common/interface/AssociationMap.h"


#include "DataFormats/CaloRecHit/interface/CaloClusterFwd.h"

#include "SimDataFormats/CaloAnalysis/interface/CaloParticleFwd.h"

////#include "SimDataFormats/Associations/interface/LayerClusterToCaloParticleAssociatorBaseImpl.h"
//#include "SimDataFormats/Associations/interface/LayerClusterToSimClusterAssociatorBaseImpl.h"

class FWCaloClusterProxyBuilder : public ROOT::Experimental::REveDataSimpleProxyBuilderTemplate<reco::CaloCluster>
{
public:
    REGISTER_FWPB_METHODS();

    using REveDataSimpleProxyBuilderTemplate<reco::CaloCluster>::BuildItem;
    virtual void BuildItem(const reco::CaloCluster &iData, int idx, ROOT::Experimental::REveElement *iItemHolder, const ROOT::Experimental::REveViewContext *context) override
    {
        auto ps = new ROOT::Experimental::REvePointSet();
        ps->SetMarkerStyle(4);
        ps->SetMarkerSize(4);

        const FWGeometry *geom = fireworks::Context::getInstance()->getGeom();

        std::vector<std::pair<DetId, float>> clusterDetIds = iData.hitsAndFractions();
        for (std::vector<std::pair<DetId, float>>::iterator it = clusterDetIds.begin(), itEnd = clusterDetIds.end();
             it != itEnd;
             ++it)
        {

            const float *corners = geom->getCorners(it->first);
            if (corners == nullptr)
                continue;
            ps->SetNextPoint(corners[0], corners[1], corners[2]);
        }
        SetupAddElement(ps, iItemHolder);

        /*
  typedef edm::AssociationMap<
      edm::OneToManyWithQualityGeneric<CaloParticleCollection, reco::CaloClusterCollection, std::pair<float, float>>>
      SimToRecoCollection;
  typedef edm::AssociationMap<
      edm::OneToManyWithQualityGeneric<reco::CaloClusterCollection, CaloParticleCollection, float>>
      RecoToSimCollection;


        const fwlite::Event *event = fireworks::Context::getInstance()->getCurrentEvent();

 fwlite::Handle<RecoToSimCollection> handle;
    handle.getByLabel(*event, "layerClusterCaloParticleAssociationProducer");

    printf("\nAssociations reco::ClusterCluster to CaloParticle  num_associations = %lu\n", handle->size());

    for (auto ii = handle->begin(); ii != handle->end(); ++ii)
    {
        auto &val = ii->val; // presumably typedef std::vector<std::pair<ValRef, Q> > val_type
        printf("reco::CaloCluster idx = %d associated with %lu CaloParicles:\n", ii->key.index(), val.size());
        for (unsigned int j = 0; j < val.size(); ++j)
        {
            auto quality = val[j].second;
            printf("CaloParticle ref idx = [%2u] qualitity=(%f)\n", val[j].first.index(), quality);
        }
        printf("\n");
    }
    */
    }
};

REGISTER_FW2PROXYBUILDER(FWCaloClusterProxyBuilder, reco::CaloCluster, "Calo Cluster");
