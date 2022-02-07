#include "FireworksWeb/Core/interface/FWProxyBuilderFactory.h"

#include "ROOT/REvePointSet.hxx"
#include "ROOT/REveViewContext.hxx"
#include "ROOT/REveDataSimpleProxyBuilderTemplate.hxx"

#include "DataFormats/CaloRecHit/interface/CaloClusterFwd.h"
#include "DataFormats/Common/interface/ValueMap.h"

class FWCaloClusterProxyBuilder : public ROOT::Experimental::REveDataSimpleProxyBuilderTemplate<reco::CaloCluster>
{
public:
    REGISTER_FWPB_METHODS();

    using REveDataSimpleProxyBuilderTemplate<reco::CaloCluster>::BuildItem;
    virtual void BuildItem(const reco::CaloCluster &iData, int /*idx*/, ROOT::Experimental::REveElement *iItemHolder, const ROOT::Experimental::REveViewContext *context) override
    {
        auto ps = new ROOT::Experimental::REvePointSet("vertex pnt");
        ps->SetNextPoint(iData.x(), iData.y(), iData.z());
        ps->SetMarkerStyle(4);
        ps->SetMarkerSize(4);




        SetupAddElement(ps, iItemHolder);
    }
};

REGISTER_FW2PROXYBUILDER(FWCaloClusterProxyBuilder, reco::CaloCluster, "Calo Cluster");
