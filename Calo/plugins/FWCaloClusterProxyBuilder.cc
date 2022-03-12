#include "FireworksWeb/Core/interface/FWProxyBuilderFactory.h"

#include "ROOT/REvePointSet.hxx"
#include "ROOT/REveBoxSet.hxx"
#include "ROOT/REveRGBAPalette.hxx"
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
#include "SimDataFormats/CaloAnalysis/interface/CaloParticle.h"


#include "DataFormats/CaloRecHit/interface/CaloClusterFwd.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/FWLite/interface/Event.h"
#include "DataFormats/FWLite/interface/Handle.h"

#include "DataFormats/Common/interface/AssociationMap.h"


#include "DataFormats/CaloRecHit/interface/CaloClusterFwd.h"

#include "SimDataFormats/CaloAnalysis/interface/CaloParticleFwd.h"


using namespace ROOT::Experimental;
class FWCaloClusterProxyBuilder : public ROOT::Experimental::REveDataSimpleProxyBuilderTemplate<reco::CaloCluster>
{
public:
    REGISTER_FWPB_METHODS();

    REveRGBAPalette *m_palette{nullptr};

    FWCaloClusterProxyBuilder()
    {
        m_palette = new REveRGBAPalette(0, 130);
        m_palette->IncRefCount();
    }

    ~FWCaloClusterProxyBuilder()
    {
        m_palette->DecRefCount();
    }

    using REveDataSimpleProxyBuilderTemplate<reco::CaloCluster>::BuildItem;
    virtual void BuildItem(const reco::CaloCluster &iData, int idx, ROOT::Experimental::REveElement *iItemHolder, const ROOT::Experimental::REveViewContext *context) override
    {   
        const FWGeometry *geom = fireworks::Context::getInstance()->getGeom();
        std::vector<std::pair<DetId, float>> clusterDetIds = iData.hitsAndFractions();

        auto ps = new REveBoxSet();
        ps->SetPalette(m_palette);
        ps->Reset(REveBoxSet::kBT_AABox, kFALSE, clusterDetIds.size());
        for (std::vector<std::pair<DetId, float>>::iterator it = clusterDetIds.begin(), itEnd = clusterDetIds.end();
             it != itEnd;
             ++it)
        {

            const float *corners = geom->getCorners(it->first);
            if (corners == nullptr)
                continue;

            float s = 1;
            ps->AddBox(corners[0], corners[1], corners[2], s, s, s);
            ps->DigitValue(4);
        }
        ps->RefitPlex();
        SetupAddElement(ps, iItemHolder);
    }
};

REGISTER_FW2PROXYBUILDER(FWCaloClusterProxyBuilder, reco::CaloCluster, "Calo Cluster");
