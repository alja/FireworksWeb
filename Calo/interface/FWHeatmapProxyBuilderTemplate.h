#ifndef FireworksWeb_Core_FWHeatmapProxyBuilderTemplate_h
#define FireworksWeb_Core_FWHeatmapProxyBuilderTemplate_h
// -*- C++ -*-
//
// Package:     Core
// Class  :     FWHeatmapProxyBuilderTemplate
//
/**\class FWHeatmapProxyBuilderTemplate FWHeatmapProxyBuilderTemplate.h Fireworks/Calo/interface/FWHeatmapProxyBuilderTemplate.h

   Description: <one line class summary>

   Usage:
    <usage>

 */
//
// Original Author:  Alex Mourtziapis
//         Created:  Wed  Jan  23 14:50:00 EST 2019
//

// system include files
#include <cmath>

// user include files

#include "ROOT/REveDataSimpleProxyBuilder.hxx"
#include "FWCore/Framework/interface/Event.h"
#include "DataFormats/DetId/interface/DetId.h"

#include "FireworksWeb/Core/interface/FWProxyBuilderConfiguration.h"
#include "FireworksWeb/Core/interface/FWWebEventItem.h"
#include "FireworksWeb/Calo/interface/hgcal_gradient.h"

// forward declarations
class HGCRecHit;

using namespace ROOT::Experimental;

template <typename T>
class FWHeatmapProxyBuilderTemplate : public REveDataSimpleProxyBuilder { 
public:
  FWHeatmapProxyBuilderTemplate() : REveDataSimpleProxyBuilder() {}

  //virtual ~FWHeatmapProxyBuilderTemplate(){}

protected:
  std::unordered_map<DetId, const HGCRecHit*> m_hitmap;

  using REveDataProxyBuilderBase::SetCollection;
  void SetCollection(REveDataCollection* c) override
  {
    fwhgcal::initProxyParameters(c);
    REveDataProxyBuilderBase::SetCollection(c);
  }

  using REveDataProxyBuilderBase::Build;
  void Build() override {
    fwhgcal::fillDetIdToHitMap(m_hitmap);
    REveDataProxyBuilderBase::Build();
  }

   using REveDataSimpleProxyBuilder::BuildItem;
   void BuildItem(const void *iData, int index, REveElement *itemHolder, const REveViewContext *context) override
   {
      if(iData) {
         BuildItem(*reinterpret_cast<const T*> (iData), index, itemHolder, context);
      }
   }

   virtual void BuildItem(const T & /*iData*/, int /*index*/, REveElement * /*itemHolder*/, const REveViewContext * /*context*/)
   {
      throw std::runtime_error("virtual Build(const T&, int, REveElement&, const REveViewContext*) not implemented by inherited class.");
   }

   using REveDataSimpleProxyBuilder::BuildItemViewType;
   void BuildItemViewType(const void *iData, int index, REveElement *itemHolder, const std::string& viewType, const REveViewContext *context) override
   {
      if(iData) {
         BuildItemViewType(*reinterpret_cast<const T*> (iData), index, itemHolder, viewType, context);
      }
   }

   virtual void BuildItemViewType(const T & /*iData*/, int /*index*/, REveElement * /*itemHolder*/, const std::string& /*viewType*/, const REveViewContext * /*context*/)
   {
      throw std::runtime_error("virtual BuildViewType(const T&, int, REveElement&, const REveViewContext*) not implemented by inherited class.");
   }

private:
  FWHeatmapProxyBuilderTemplate(const FWHeatmapProxyBuilderTemplate&) = delete;  // stop default

  const FWHeatmapProxyBuilderTemplate& operator=(const FWHeatmapProxyBuilderTemplate&) = delete;  // stop default
  // ---------- member data --------------------------------
};

#endif
