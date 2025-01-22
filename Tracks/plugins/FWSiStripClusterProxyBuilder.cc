#ifndef FireworksWeb_Tracks_FWSiStripCluster_h
#define FireworksWeb_Tracks_FWSiStripCluster_h
// -*- C++ -*-
//
// Package:     Tracks
// Class  :     FWSiStripClusterProxyBuilder
//
//

#include "ROOT/REvePointSet.hxx"
#include "ROOT/REveStraightLineSet.hxx"
#include "ROOT/REveCompound.hxx"
#include "ROOT/REveDataSimpleProxyBuilder.hxx"
#include "ROOT/REveGeoShape.hxx"
#include "ROOT/REveLine.hxx"
#include "FireworksWeb/Core/interface/FWProxyBuilderFactory.h"
#include "FireworksWeb/Core/interface/FWWebEventItem.h"
#include "FireworksWeb/Core/interface/FWGeometry.h"
#include "FireworksWeb/Core/interface/fwLog.h"
#include "FireworksWeb/Core/interface/FWWebEventItem.h"
#include "FireworksWeb/Tracks/interface/TrackUtils.h"

#include "DataFormats/SiStripCluster/interface/SiStripCluster.h"
#include "DataFormats/Common/interface/DetSetVectorNew.h"

using namespace ROOT::Experimental;

class  FWSiStripClusterProxyBuilder: public REveDataSimpleProxyBuilder
{
public:
  REGISTER_FWPB_METHODS();
  FWSiStripClusterProxyBuilder(void) {}

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

    item->SetMainTransparency(70);

   const edmNew::DetSetVector<SiStripCluster>* clusters = nullptr;
   item->get(clusters);
    if (!clusters)
    {
      return;
    }

    const FWGeometry *geom = context->getGeom();
    for (edmNew::DetSetVector<SiStripCluster>::const_iterator set = clusters->begin(), setEnd = clusters->end(); set != setEnd;
         ++set)
    {
         unsigned int id = set->detId();
         if (!geom->contains(id))
         {
            fwLog(fwlog::kWarning) << "failed get geometry of SiStripCluster with detid: " << id << std::endl;
            continue;
         }
/*
         const FWGeometry::GeomDetInfo& info = geom->find(id);

         double array[16] = {info.matrix[0],
                           info.matrix[3],
                           info.matrix[6],
                           0.,
                           info.matrix[1],
                           info.matrix[4],
                           info.matrix[7],
                           0.,
                           info.matrix[2],
                           info.matrix[5],
                           info.matrix[8],
                           0.,
                           info.translation[0],
                           info.translation[1],
                           info.translation[2],
                           1.};
*/
         // note TEveGeoShape owns shape
      
         REveGeoShape *rg = nullptr;
         bool addGeoShape = false;

         try {
               rg = geom->getEveShape(id);
               rg->SetMainTransparency(80);
               rg->SetPickable(false);
               
         }
         catch (std::exception& e) { printf("Can't access geo idx %d \n", index);}


         for (edmNew::DetSet<SiStripCluster>::const_iterator ic = set->begin(); ic != set->end(); ++ic )
         {
            REveElement *itemHolder = GetHolder(product, index);

            if (!addGeoShape && rg !=nullptr )
            {
               SetupAddElement(rg, itemHolder);
               addGeoShape = true;
            }
            
            REveLine* line = new REveLine();
            SetupAddElement(line, itemHolder);
            float localTop[3] = {0.0, 0.0, 0.0};
            float localBottom[3] = {0.0, 0.0, 0.0};
            fireworks::localSiStrip((*ic).firstStrip(), localTop, localBottom, geom->getParameters(id), id);
            float globalTop[3];
            float globalBottom[3];
            geom->localToGlobal(id, localTop, globalTop, localBottom, globalBottom);
            line->SetPoint(0, globalTop[0], globalTop[1], globalTop[2]);
            line->SetPoint(1, globalBottom[0], globalBottom[1], globalBottom[2]);

            index++;

         }
         if (product)
           printf("product children %d \n", product->NumChildren());
       }
  }
/*

void ModelChanges(const REveDataCollection::Ids_t &ids, Product *product)
{
  // We know there is only one element in this product
  printf("FWDigitSetProxyBuilder::ModelChanges %zu\n", ids.size());

  for (auto &i : ids)
  {
    auto item = Collection()->GetDataItem(i);
    m_boxSet->SetCurrentDigit(i);
    if (item->GetVisible())
    {
      m_boxSet->DigitValue(1);
      m_boxSet->DigitColor(item->GetMainColor());
    }
    else
    {
      m_boxSet->DigitValue(0);
    }
  }

  m_boxSet->StampObjProps();
}*/


};

REGISTER_FW2PROXYBUILDER_BASE(FWSiStripClusterProxyBuilder, edmNew::DetSetVector<SiStripCluster>, "SiStripCluster");
#endif
