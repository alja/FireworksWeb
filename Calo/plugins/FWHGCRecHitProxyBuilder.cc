#include "FireworksWeb/Core/interface/FWProxyBuilderFactory.h"

#include "ROOT/REvePointSet.hxx"
#include "ROOT/REveViewContext.hxx"
#include "ROOT/REveDataSimpleProxyBuilderTemplate.hxx"

#include "FireworksWeb/Core/interface/FWGeometry.h"
#include "FireworksWeb/Core/interface/fwLog.h"
#include "FireworksWeb/Core/interface/Context.h"

#include "DataFormats/HGCRecHit/interface/HGCRecHitCollections.h"

class FWHGCRecHitProxyBuilder : public ROOT::Experimental::REveDataSimpleProxyBuilderTemplate<HGCRecHit>
{
public:
    REGISTER_FWPB_METHODS();

    using REveDataSimpleProxyBuilderTemplate<HGCRecHit>::BuildItem;
    virtual void BuildItem(const HGCRecHit &iData, int idx, ROOT::Experimental::REveElement *iItemHolder, const ROOT::Experimental::REveViewContext *context) override
    {
        auto ps = new ROOT::Experimental::REvePointSet();
        ps->SetMarkerStyle(4);
        ps->SetMarkerSize(4);

        const FWGeometry *geom = fireworks::Context::getInstance()->getGeom();

        unsigned int ID = iData.id();

        //const bool z = (ID >> 25) & 0x1;

        const float *corners = geom->getCorners(ID);

        ps->SetNextPoint(corners[0], corners[1], corners[2]);

      printf("HGCAL idx = %d corners (%f, %f, %f)\n", idx, corners[0], corners[1], corners[2]);
        SetupAddElement(ps, iItemHolder);
    }
};

REGISTER_FW2PROXYBUILDER(FWHGCRecHitProxyBuilder, HGCRecHit, "HGC RecHit");
