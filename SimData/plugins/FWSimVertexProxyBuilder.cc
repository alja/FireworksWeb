/*
 *  FWSimVertexProxyBuilder.cc
 *  FWorks
 *
 *  Created by Ianna Osborne on 9/9/10.
 *
 */

#include "FireworksWeb/Core/interface/FWProxyBuilderFactory.h"
#include "SimDataFormats/Vertex/interface/SimVertex.h"

#include "ROOT/REvePointSet.hxx"
#include "ROOT/REveDataSimpleProxyBuilderTemplate.hxx"

using namespace ROOT::Experimental;

class FWSimVertexProxyBuilder : public REveDataSimpleProxyBuilderTemplate<SimVertex> {
public:
  FWSimVertexProxyBuilder(void) {}
  ~FWSimVertexProxyBuilder(void) override {}

   REGISTER_FWPB_METHODS();

   using REveDataSimpleProxyBuilderTemplate<SimVertex>::BuildItem;
   virtual void BuildItem(const SimVertex& iData, int /*idx*/, REveElement* iItemHolder, const REveViewContext* vc) override
   { 
     REvePointSet* pointSet = new REvePointSet;

      pointSet->SetMarkerStyle(4);
      pointSet->SetMarkerSize(4);
     pointSet->SetNextPoint(iData.position().x(), iData.position().y(), iData.position().z());
     SetupAddElement(pointSet, iItemHolder);
   }
};

REGISTER_FW2PROXYBUILDER(FWSimVertexProxyBuilder, SimVertex, "SimVertices");