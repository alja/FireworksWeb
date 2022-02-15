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

#include "DataFormats/Common/interface/AssociationMap.h"
#include "DataFormats/Common/interface/ValueMap.h"

#include "DataFormats/FWLite/interface/Event.h"
#include "DataFormats/FWLite/interface/Handle.h"

#include "DataFormats/Common/interface/AssociationMap.h"
#include "DataFormats/CaloRecHit/interface/CaloClusterFwd.h"

#include "SimDataFormats/CaloAnalysis/interface/CaloParticle.h"
#include "SimDataFormats/CaloAnalysis/interface/CaloParticleFwd.h"


#include "SimDataFormats/Associations/interface/LayerClusterToCaloParticleAssociatorBaseImpl.h"
//#include "SimDataFormats/Associations/interface/LayerClusterToSimClusterAssociatorBaseImpl.h"

class FWClusterCaloParticleAssociationProxyBuilder : public ROOT::Experimental::REveDataProxyBuilderBase
{
public:
    REGISTER_FWPB_METHODS();

    using ROOT::Experimental::REveDataProxyBuilderBase::Build;
    virtual void Build() override
    {
        print();
    }

    void print()
    {
        const fwlite::Event *event = fireworks::Context::getInstance()->getCurrentEvent();

        fwlite::Handle<hgcal::RecoToSimCollection> handle;
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
    }

    using ROOT::Experimental::REveDataProxyBuilderBase::FillImpliedSelected;
    void FillImpliedSelected(ROOT::Experimental::REveElement::Set_t &impSet, Product *p) override
    {
        // printf("RecHit fill implioed ----------------- !!!%zu\n", Collection()->GetItemList()->RefSelectedSet().size());
    }

    using ROOT::Experimental::REveDataProxyBuilderBase::ModelChanges;
    void ModelChanges(const ROOT::Experimental::REveDataCollection::Ids_t &ids, Product *product) override
    {
        // We know there is only one element in this product
        //  printf("RecHitProxyBuilder::model changes %zu\n", ids.size());
    }
};

REGISTER_FW2PROXYBUILDER(FWClusterCaloParticleAssociationProxyBuilder, hgcal::RecoToSimCollection, "TestAssociations");
