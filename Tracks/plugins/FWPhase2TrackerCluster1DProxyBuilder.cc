#ifndef FireworksWeb_Tracks_FWPhase2TrackerCluster1D_h
#define FireworksWeb_Tracks_FWPhase2TrackerCluster1D_h
// -*- C++ -*-
//
// Package:     Tracks
// Class  :     FWPhase2TrackerCluster1DProxyBuilder
//
//

#include "ROOT/REvePointSet.hxx"
#include "ROOT/REveStraightLineSet.hxx"
#include "ROOT/REveCompound.hxx"
#include "ROOT/REveDataSimpleProxyBuilder.hxx"
#include "ROOT/REveGeoShape.hxx"
#include "FireworksWeb/Core/interface/FWProxyBuilderFactory.h"
#include "FireworksWeb/Core/interface/FWWebEventItem.h"
#include "FireworksWeb/Core/interface/FWGeometry.h"
#include "FireworksWeb/Core/interface/fwLog.h"
#include "FireworksWeb/Core/interface/FWWebEventItem.h"
#include "FireworksWeb/Tracks/interface/TrackUtils.h"

#include "DataFormats/Phase2TrackerCluster/interface/Phase2TrackerCluster1D.h"

using namespace ROOT::Experimental;

class FWPhase2TrackerCluster1DProxyBuilder : public REveDataSimpleProxyBuilder
{
public:
  REGISTER_FWPB_METHODS();
  FWPhase2TrackerCluster1DProxyBuilder(void) {}

  using REveDataSimpleProxyBuilder::BuildItem;
  void BuildItem(const void *, int, REveElement *, const REveViewContext *) {}

  using REveDataSimpleProxyBuilder::BuildItemViewType;
  void BuildItemViewType(const void *, int, REveElement *, const std::string &, const REveViewContext *) {}

  using REveDataSimpleProxyBuilder::BuildProduct;
  void BuildProduct(const REveDataCollection *collection, REveElement *product, const REveViewContext *) override
  {
    int index = 0;
    auto context = fireworks::Context::getInstance();
    FWWebEventItem *item = dynamic_cast<FWWebEventItem*>(Collection());

  const Phase2TrackerCluster1DCollectionNew* pixels = nullptr;
  item->get(pixels);
    if (!pixels)
    {
      return;
    }

    const FWGeometry *geom = context->getGeom();
    for (Phase2TrackerCluster1DCollectionNew::const_iterator set = pixels->begin(), setEnd = pixels->end(); set != setEnd;
         ++set)
    {
      unsigned int id = set->detId();
      if (!geom->contains(id))
      {
        fwLog(fwlog::kWarning) << "failed get geometry of Phase2TrackerCluster1D with detid: " << id << std::endl;
        continue;
      }
      const float *pars = geom->getParameters(id);
      const float *shape = geom->getShapePars(id);

      REveGeoShape *rg = geom->getEveShape(id);
      rg->SetMainTransparency(50);
      rg->SetPickable(false);
      bool addGeoShape = false;

      const edmNew::DetSet<Phase2TrackerCluster1D> &clusters = *set;

      for (edmNew::DetSet<Phase2TrackerCluster1D>::const_iterator itc = clusters.begin(), edc = clusters.end();
           itc != edc;
           ++itc)
      {
        REveElement *itemHolder = GetHolder(product, index);

        if (!addGeoShape)
        {
          SetupAddElement(rg, itemHolder);
          addGeoShape = true;
        }
        float halfLength = shape[2];
        float pitchSecond = pars[1];

        // line
        //
        REveStraightLineSet *lineSet = new REveStraightLineSet;
        float localPointBeg[3] = {fireworks::phase2PixelLocalX((*itc).center(), pars, shape),
                                  float((*itc).column()) * pitchSecond - halfLength,
                                  0.0};
        float localPointEnd[3] = {fireworks::phase2PixelLocalX((*itc).center(), pars, shape),
                                  float((*itc).column() + 1.0) * pitchSecond - halfLength,
                                  0.0};

        float globalPointBeg[3];
        float globalPointEnd[3];
        geom->localToGlobal(id, localPointBeg, globalPointBeg);
        geom->localToGlobal(id, localPointEnd, globalPointEnd);

        lineSet->AddLine(globalPointBeg, globalPointEnd);
        lineSet->AddMarker(0, 0.5f);
        lineSet->SetMarkerStyle(1);

        SetupAddElement(lineSet, itemHolder);
        index++;
      }
    }
  }
};

REGISTER_FW2PROXYBUILDER_BASE(FWPhase2TrackerCluster1DProxyBuilder, Phase2TrackerCluster1DCollectionNew, "Phase2TrackerCluster1D");
#endif
