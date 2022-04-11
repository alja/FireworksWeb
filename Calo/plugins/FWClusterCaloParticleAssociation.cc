#include "FireworksWeb/Core/interface/FWAssociationFactory.h"
#include "FireworksWeb/Core/interface/FWAssociationProxyBase.h"

#include "ROOT/REvePointSet.hxx"
#include "ROOT/REveViewContext.hxx"
#include "ROOT/REveDataSimpleProxyBuilderTemplate.hxx"

#include "FireworksWeb/Core/interface/fwLog.h"
#include "FireworksWeb/Core/interface/Context.h"
#include "FireworksWeb/Core/interface/FWEveAssociation.h"

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

class FWClusterCaloParticleAssociation : public FWAssociationProxyBase
{
public:
    REGISTER_FWASSOCIATION_METHODS();
    FWClusterCaloParticleAssociation() {
        std::cout << "creating caling FWClusterCaloParticleAssociation() \n";
    }

    ~FWClusterCaloParticleAssociation() override{};

    using FWAssociationProxyBase::getIndices;
    virtual void getIndices(std::set<int> &inSet, std::set<int> &outSet) const
    {
        hgcal::RecoToSimCollection *handle = reinterpret_cast<hgcal::RecoToSimCollection *>(getEveObj()->data());

        /*
        printf("\nAssociations reco::ClusterCluster to CaloParticle  num_associations = %lu\n", handle->size());

        for (auto &i: inSet){
            std::cout << "FWClusterCaloParticleAssociation inputIndices ...in index " << i << "\n";
        }
*/
        for (auto ii = handle->begin(); ii != handle->end(); ++ii)
        {
            auto &val = ii->val; // presumably typedef std::vector<std::pair<ValRef, Q> > val_type
            if (inSet.find(ii->key.index()) != inSet.end())
            {
                for (unsigned int j = 0; j < val.size(); ++j)
                {
                    auto quality = val[j].second;
                    if (getEveObj()->filterPass(quality))
                    {
                        printf("CaloParticle ref idx = [%2u] qualitity=(%f)\n", val[j].first.index(), quality);
                        outSet.insert(val[j].first.index());
                    }
                }
            }
        }
    }
};

//___________________________________________________________________________________________________________
//___________________________________________________________________________________________________________
//___________________________________________________________________________________________________________
//___________________________________________________________________________________________________________

class FWCaloParticleClusterAssociation : public FWAssociationProxyBase
{
public:
    REGISTER_FWASSOCIATION_METHODS();
    FWCaloParticleClusterAssociation()
    {
        std::cout << "creating caling FWClusterCaloParticleAssociation() \n";
    }

    using FWAssociationProxyBase::getIndices;
    virtual void getIndices(std::set<int> &inSet, std::set<int> &outSet) const
    {

        hgcal::SimToRecoCollection *handle = reinterpret_cast<hgcal::SimToRecoCollection *>(getEveObj()->data());
        /*
        printf("\nAssociations CaloParticle to reco::CaloCluster  num_associations = %lu\n", handle->size());

        for (auto &i : inSet)
        {
            std::cout << "FWCaloParticleClusterAssociation input indices ...in index " << i << "\n";
        }
*/
        for (auto ii = handle->begin(); ii != handle->end(); ++ii)
        {
            auto &val = ii->val; // presumably typedef std::vector<std::pair<ValRef, Q> > val_type
            if (inSet.find(ii->key.index()) != inSet.end())
            {
                for (unsigned int j = 0; j < val.size(); ++j)
                {
                    auto quality = val[j].second;
                    if (getEveObj()->filterPass(quality))
                    {
                        printf("CaloParticle ref idx = [%2u] qualitity=(%f, %f)\n", val[j].first.index(), quality.first, quality.second);
                        outSet.insert(val[j].first.index());
                    }
                }
            }
        }
    }
};

REGISTER_FWASSOCIATION(FWClusterCaloParticleAssociation, hgcal::RecoToSimCollection, "HGCAL_ClusterToParticle", "reco::CaloCluster", "CaloParticle");
REGISTER_FWASSOCIATION(FWCaloParticleClusterAssociation, hgcal::SimToRecoCollection, "HGCAL_ParticleToCluster", "CaloParticle", "reco::CaloCluster");
