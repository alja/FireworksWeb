#include "FireworksWeb/Core/interface/FWProxyBuilderFactory.h"
#include "FireworksWeb/Calo/interface/FWHeatmapProxyBuilderTemplate.h"

#include "ROOT/REvePointSet.hxx"
#include "ROOT/REveBoxSet.hxx"
#include "ROOT/REveViewContext.hxx"
#include "ROOT/REveStraightLineSet.hxx"
#include "ROOT/REveDataSimpleProxyBuilderTemplate.hxx"

#include "FireworksWeb/Core/interface/FWGeometry.h"
#include "FireworksWeb/Core/interface/fwLog.h"
#include "FireworksWeb/Core/interface/Context.h"
#include "FireworksWeb/Core/interface/BuilderUtils.h"

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
#include "DataFormats/HGCRecHit/interface/HGCRecHitCollections.h"

#include "SimDataFormats/CaloAnalysis/interface/CaloParticleFwd.h"


using namespace ROOT::Experimental;
class FWCaloClusterProxyBuilder : public FWHeatmapProxyBuilderTemplate<reco::CaloCluster>
{
    public:
    REGISTER_FWPB_METHODS();

    using FWHeatmapProxyBuilderTemplate::SetCollection;
    void SetCollection(REveDataCollection* c) override {
        auto item = dynamic_cast<FWWebEventItem *>(c);
        item->getConfig()->assertParam("Cluster(0)/RecHit(1)", false);
        item->getConfig()->assertParam("EnableTimeFilter", false);
        item->getConfig()->assertParam("TimeLowerBound(ns)", 0.01, 0.0, 75.0);
        item->getConfig()->assertParam("TimeUpperBound(ns)", 0.01, 0.0, 75.0);
        FWHeatmapProxyBuilderTemplate::SetCollection(c);
    }

    using FWHeatmapProxyBuilderTemplate<reco::CaloCluster>::Build;
    void Build() override;

    using FWHeatmapProxyBuilderTemplate<reco::CaloCluster>::BuildItem;
    virtual void BuildItem(const reco::CaloCluster &iData, int idx, ROOT::Experimental::REveElement *iItemHolder, const ROOT::Experimental::REveViewContext *context) override;

private:
    edm::Handle<edm::ValueMap<float>> TimeValueMapHandle;
    double timeLowerBound, timeUpperBound;
    long layer;
    double saturation_energy;
    bool heatmap;
    bool z_plus;
    bool z_minus;
    bool enableTimeFilter;
};

//------------------------------------------------------------
void FWCaloClusterProxyBuilder::Build() {
  auto fwitem = dynamic_cast<FWWebEventItem*>(Collection());
  auto event = fireworks::Context::getInstance()->getCurrentEvent();
  event->getByLabel(edm::InputTag("hgcalLayerClusters", "timeLayerCluster"), TimeValueMapHandle);
  if (TimeValueMapHandle.isValid()) {
    timeLowerBound = std::min(fwitem->getConfig()->value<double>("TimeLowerBound(ns)"),
                              fwitem->getConfig()->value<double>("TimeUpperBound(ns)"));
    timeUpperBound = std::max(fwitem->getConfig()->value<double>("TimeLowerBound(ns)"),
                              fwitem->getConfig()->value<double>("TimeUpperBound(ns)"));
  } else {
    event->getByLabel(edm::InputTag("hgcalMergeLayerClusters", "timeLayerCluster"), TimeValueMapHandle);
    std::cerr << __FILE__ << ":" << __LINE__
              << " couldn't locate 'hgcalLayerClusters:timeLayerCluster' ValueMap in input file. Trying to access "
                 "'hgcalMergeLayerClusters:timeLayerClusters' ValueMap"
              << std::endl;
    if (!TimeValueMapHandle.isValid()) {
      std::cerr << __FILE__ << ":" << __LINE__
                << " couldn't locate 'hgcalMergeLayerClusters:timeLayerCluster' ValueMap in input file." << std::endl;
    }
  }

  layer = fwitem->getConfig()->value<long>("Layer");
  saturation_energy = fwitem->getConfig()->value<double>("EnergyCutOff");
  heatmap = fwitem->getConfig()->value<bool>("Heatmap");
  z_plus = fwitem->getConfig()->value<bool>("Z+");
  z_minus = fwitem->getConfig()->value<bool>("Z-");
  enableTimeFilter = fwitem->getConfig()->value<bool>("EnableTimeFilter");

  FWHeatmapProxyBuilderTemplate::Build();
}

//------------------------------------------------------------
void FWCaloClusterProxyBuilder::BuildItem(const reco::CaloCluster &iData,
                                          int idx, ROOT::Experimental::REveElement *oItemHolder, 
                                          const ROOT::Experimental::REveViewContext *context)
{
  if (enableTimeFilter && TimeValueMapHandle.isValid()) {
    const float time = TimeValueMapHandle->get(idx);
    if (time < timeLowerBound || time > timeUpperBound)
      return;
  }

  std::vector<std::pair<DetId, float>> clusterDetIds = iData.hitsAndFractions();
  
  Color_t itemColor = Collection()->GetDataItem(idx)->GetMainColor();

  bool h_hex(false);
  REveBoxSet *hex_boxset = new REveBoxSet();
  hex_boxset->Reset(REveBoxSet::kBT_InstancedScaledRotated, true, 64);
  if (!heatmap){
    hex_boxset->UseSingleColor();
    hex_boxset->SetMainColorPtr(new Color_t);
    hex_boxset->SetMainColor(itemColor);
  }
  hex_boxset->SetPickable(true);
  hex_boxset->SetAntiFlick(true);

  bool h_box(false);
  REveBoxSet *boxset = new REveBoxSet();
  boxset->Reset(REveBoxSet::kBT_FreeBox, true, 64);
  if (!heatmap) {
    boxset->UseSingleColor();
    boxset->SetMainColorPtr(new Color_t);
    boxset->SetMainColor(itemColor);
  }
  boxset->SetPickable(true);
  boxset->SetAntiFlick(true);

  auto fwitem = dynamic_cast<FWWebEventItem *>(Collection());
  auto geom = fireworks::Context::getInstance()->getGeom();

  for (std::vector<std::pair<DetId, float>>::iterator it = clusterDetIds.begin(), itEnd = clusterDetIds.end();
       it != itEnd;
       ++it) {
    const uint8_t type = ((it->first >> 28) & 0xF);

    const float *corners = geom->getCorners(it->first);
    if (corners == nullptr)
      continue;

    // HGCal
    if (iData.algo() == 8 || (type >= 8 && type <= 10)) {
      if (heatmap && m_hitmap.find(it->first) == m_hitmap.end())
        continue;

      const bool z = (it->first >> 25) & 0x1;

      // discard everything thats not at the side that we are intersted in
      if (((z_plus & z_minus) != 1) && (((z_plus | z_minus) == 0) || !(z == z_minus || z == !z_plus)))
        continue;

      const float *parameters = geom->getParameters(it->first);
      const float *shapes = geom->getShapePars(it->first);

      if (parameters == nullptr || shapes == nullptr)
        continue;

      const int total_points = parameters[0];
      const bool isScintillator = (total_points == 4);

      uint8_t ll = layer;
      if (layer > 0) {
        if (layer > 28) {
          if (type == 8) {
            continue;
          }
          ll -= 28;
        } else {
          if (type != 8) {
            continue;
          }
        }

        if (ll != ((it->first >> (isScintillator ? 17 : 20)) & 0x1F))
          continue;
      }

      // seed
      if (iData.seed().rawId() == it->first.rawId()) {
        REveStraightLineSet *marker = new REveStraightLineSet;
        marker->SetLineWidth(1);

        // center of RecHit
        const float center[3] = {corners[total_points * 3 + 0],
                                 corners[total_points * 3 + 1],
                                 corners[total_points * 3 + 2] + shapes[3] * 0.5f};

        // draw 3D cross
        const float crossScale = 1.0f + fmin(iData.energy(), 5.0f);
        marker->AddLine(center[0] - crossScale, center[1], center[2], center[0] + crossScale, center[1], center[2]);
        marker->AddLine(center[0], center[1] - crossScale, center[2], center[0], center[1] + crossScale, center[2]);
        marker->AddLine(center[0], center[1], center[2] - crossScale, center[0], center[1], center[2] + crossScale);

       // oItemHolder.AddElement(marker);
        SetupAddElement(marker, oItemHolder);
      }

      const float energy = fmin(
          (fwitem->getConfig()->value<bool>("Cluster(0)/RecHit(1)") ? m_hitmap[it->first]->energy() : iData.energy()) /
              saturation_energy,
          1.0f);
      const uint8_t colorFactor = FWHGCAL_GRADIENT_STEPS * energy;

      // Scintillator
      if (isScintillator) {
        const int total_vertices = 3 * total_points;

        std::vector<float> pnts(24);
        for (int i = 0; i < total_points; ++i) {
          pnts[i * 3 + 0] = corners[i * 3];
          pnts[i * 3 + 1] = corners[i * 3 + 1];
          pnts[i * 3 + 2] = corners[i * 3 + 2];

          pnts[(i * 3 + 0) + total_vertices] = corners[i * 3];
          pnts[(i * 3 + 1) + total_vertices] = corners[i * 3 + 1];
          pnts[(i * 3 + 2) + total_vertices] = corners[i * 3 + 2] + shapes[3];
        }
        boxset->AddBox(&pnts[0]);
        if (heatmap) {
          energy ? boxset->DigitColor(fwhgcal::gradient[0][colorFactor], fwhgcal::gradient[1][colorFactor], fwhgcal::gradient[2][colorFactor])
                 : boxset->DigitColor(64, 64, 64);
        }

        h_box = true;
      }
      // Silicon
      else {
        constexpr int offset = 9;

        float centerX = (corners[6] + corners[6 + offset]) / 2;
        float centerY = (corners[7] + corners[7 + offset]) / 2;
        float radius = fabs(corners[6] - corners[6 + offset]) / 2;
        hex_boxset->AddHex(REveVector(centerX, centerY, corners[2]), radius, 90.0, shapes[3]);
        if (heatmap) {
          energy ? hex_boxset->DigitColor(fwhgcal::gradient[0][colorFactor], fwhgcal::gradient[1][colorFactor], fwhgcal::gradient[2][colorFactor])
                 : hex_boxset->DigitColor(64, 64, 64);
        }

        h_hex = true;
      }
    }
    // Not HGCal
    else
    {
      h_box = true;

      std::vector<float> pnts(24);
      fireworks::energyTower3DCorners(corners, (*it).second, pnts);
      boxset->AddBox(&pnts[0]);
      if (heatmap)
        boxset->DigitColor(64, 64, 64);
    }
  }

  if (h_hex)
  {
    hex_boxset->RefitPlex();
    SetupAddElement(hex_boxset, oItemHolder);
  }

  if (h_box)
  {
    boxset->RefitPlex();
  }
  SetupAddElement(boxset, oItemHolder);
}


REGISTER_FW2PROXYBUILDER(FWCaloClusterProxyBuilder, reco::CaloCluster, "Calo Cluster");
