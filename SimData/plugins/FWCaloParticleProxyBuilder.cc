#include "FireworksWeb/Core/interface/FWProxyBuilderFactory.h"

#include "ROOT/REvePointSet.hxx"
#include "ROOT/REveBoxSet.hxx"
#include "ROOT/REveViewContext.hxx"
#include "ROOT/REveDataSimpleProxyBuilderTemplate.hxx"

#include "FireworksWeb/Core/interface/FWGeometry.h"
#include "FireworksWeb/Core/interface/fwLog.h"
#include "FireworksWeb/Core/interface/Context.h"


#include "SimDataFormats/CaloAnalysis/interface/CaloParticle.h"
#include "SimDataFormats/CaloAnalysis/interface/CaloParticleFwd.h"
#include "SimDataFormats/CaloAnalysis/interface/SimCluster.h"


class FWCaloParticleProxyBuilder : public ROOT::Experimental::REveDataSimpleProxyBuilderTemplate<CaloParticle>
{
public:
    REGISTER_FWPB_METHODS();

    using REveDataSimpleProxyBuilderTemplate<CaloParticle>::BuildItem;
    virtual void BuildItem(const CaloParticle &iData, int /*idx*/, ROOT::Experimental::REveElement *iItemHolder, const ROOT::Experimental::REveViewContext *context) override
    {

        const FWGeometry *geom = fireworks::Context::getInstance()->getGeom();
        auto ps = new ROOT::Experimental::REvePointSet();
        ps->SetMarkerStyle(4);
        ps->SetMarkerSize(4);
        for (const auto &c : iData.simClusters())
        {
            for (const auto &it : (*c).hits_and_fractions())
            {
                // const bool z = (it.first >> 25) & 0x1;

                // discard everything thats not at the side that we are intersted in
                // if (((z_plus & z_minus) != 1) && (((z_plus | z_minus) == 0) || !(z == z_minus || z == !z_plus)))
                //     continue;

                try
                {
                    const float *corners = geom->getCorners(it.first);
if (corners)
                    ps->SetNextPoint(corners[0], corners[1], corners[2]);
                }
                catch (std::exception &e)
                {
                    fwLog(fwlog::kError) << "FWCaloParticleProxyBuilder " << e.what() << "\n";
                }
            }
        }
        SetupAddElement(ps, iItemHolder);
    }
};

REGISTER_FW2PROXYBUILDER(FWCaloParticleProxyBuilder, CaloParticle, "CaloParticle");


/*
        ROOT::Experimental::REveBoxSet *boxset = new ROOT::Experimental::REveBoxSet();
        boxset->UseSingleColor();
        boxset->SetPickable(true);
        boxset->Reset(ROOT::Experimental::REveBoxSet::kBT_FreeBox, true, 64);
        boxset->SetAntiFlick(true);

        for (const auto &c : iData.simClusters())
        {
            for (const auto &it : (*c).hits_and_fractions())
            {
                if (DetId(it.first).det() != DetId::Detector::Ecal)
                {
                    std::cerr << "this proxy should be used only for ECAL";
                   // return;
                }

                const float *corners = geom->getCorners(it.first);
                if (corners == nullptr)
                {
                    std::cerr << "Det it nit fount " << DetId(it.first).det() << "\n";
                    continue;
                }
                boxset->AddBox(corners);
            }
        }

        boxset->RefitPlex();
        boxset->CSCApplyMainColorToMatchingChildren();
        boxset->CSCApplyMainTransparencyToMatchingChildren();
        SetupAddElement(boxset, iItemHolder);
        */
