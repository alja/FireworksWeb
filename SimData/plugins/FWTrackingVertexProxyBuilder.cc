/*
 *  FWTrackingVertexProxyBuilder.cc
 *  FWorks
 *
 *  Created by Ianna Osborne on 10/6/10.
 *
 */

#include "FireworksWeb/Core/interface/Context.h"
#include "SimDataFormats/TrackingAnalysis/interface/TrackingVertex.h"
#include "FireworksWeb/Core/interface/FWProxyBuilderFactory.h"

#include "ROOT/REveDataSimpleProxyBuilderTemplate.hxx"
#include "ROOT/REvePointSet.hxx"

using namespace ROOT::Experimental;

class FWTrackingVertexProxyBuilder : public REveDataSimpleProxyBuilderTemplate<TrackingVertex>
{
public:
   REGISTER_FWPB_METHODS();

   using REveDataSimpleProxyBuilderTemplate<TrackingVertex>::BuildItem;
   virtual void BuildItem(const TrackingVertex& iData, int /*idx*/, REveElement* iItemHolder, const REveViewContext* vc) override
   {
      REvePointSet* pointSet = new REvePointSet;
      pointSet->SetNextPoint(iData.position().x(), iData.position().y(), iData.position().z());
      SetupAddElement(pointSet, iItemHolder);
   }
};


REGISTER_FW2PROXYBUILDER(FWTrackingVertexProxyBuilder, TrackingVertex, "TrackingVertex");