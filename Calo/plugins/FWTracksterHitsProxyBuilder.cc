#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "DataFormats/HGCalReco/interface/Trackster.h"
#include "DataFormats/CaloRecHit/interface/CaloCluster.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/ForwardDetId/interface/HGCSiliconDetId.h"
#include "DataFormats/ForwardDetId/interface/HGCScintillatorDetId.h"
#include "DataFormats/FWLite/interface/Event.h"
#include "DataFormats/HGCRecHit/src/HGCRecHit.cc"

#include "FireworksWeb/Core/interface/FWProxyBuilderFactory.h"
#include "FireworksWeb/Calo/interface/FWHeatmapProxyBuilderTemplate.h"
#include "FireworksWeb/Core/interface/FWWebEventItem.h"
#include "FireworksWeb/Core/interface/FWGeometry.h"
#include "FireworksWeb/Core/interface/BuilderUtils.h"

#include "ROOT/REveVector.hxx"
#include "ROOT/REveBoxSet.hxx"
#include "ROOT/REveStraightLineSet.hxx"

using namespace ROOT::Experimental;

class FWTracksterHitsProxyBuilder : public FWHeatmapProxyBuilderTemplate<ticl::Trackster> {
public:
  FWTracksterHitsProxyBuilder(void) {}
 // ~FWTracksterHitsProxyBuilder(void) override {}

  REGISTER_FWPB_METHODS();

  FWTracksterHitsProxyBuilder(const FWTracksterHitsProxyBuilder &) = delete;                   // stop default
  const FWTracksterHitsProxyBuilder &operator=(const FWTracksterHitsProxyBuilder &) = delete;  // stop default

private:
  edm::Handle<edm::ValueMap<std::pair<float, float>>> TimeValueMapHandle_;
  edm::Handle<std::vector<reco::CaloCluster>> layerClustersHandle_;
  double timeLowerBound_, timeUpperBound_;
  long layer_;
  double saturation_energy_;
  bool heatmap_;
  bool z_plus_;
  bool z_minus_;
  bool enableTimeFilter_;
  bool enableSeedLines_;
  bool enablePositionLines_;
  bool enableEdges_;

using FWHeatmapProxyBuilderTemplate::SetCollection;
void SetCollection(REveDataCollection* c) override {
    auto item = dynamic_cast<FWWebEventItem *>(c);
    item->getConfig()->assertParam("Cluster(0)/RecHit(1)", false);
    item->getConfig()->assertParam("EnableSeedLines", false);
    item->getConfig()->assertParam("EnablePositionLines", false);
    item->getConfig()->assertParam("EnableEdges", false);
    item->getConfig()->assertParam("EnableTimeFilter", false);
    item->getConfig()->assertParam("TimeLowerBound(ns)", 0.01, 0.0, 75.0);
    item->getConfig()->assertParam("TimeUpperBound(ns)", 0.01, 0.0, 75.0);

    FWHeatmapProxyBuilderTemplate::SetCollection(c);
  }

  using FWHeatmapProxyBuilderTemplate<ticl::Trackster>::Build;
  void Build() override;

  using FWHeatmapProxyBuilderTemplate<ticl::Trackster>::BuildItem;
  virtual void BuildItem(const ticl::Trackster& iData, int /*idx*/, REveElement* iItemHolder, const REveViewContext* vc) override;
};

void FWTracksterHitsProxyBuilder::Build() {
  auto fwitem = dynamic_cast<FWWebEventItem*>(Collection());
  auto event = fireworks::Context::getInstance()->getCurrentEvent();
  event->getByLabel(edm::InputTag("hgcalLayerClusters", "timeLayerCluster"), TimeValueMapHandle_);
  event->getByLabel(edm::InputTag("hgcalLayerClusters"), layerClustersHandle_);
  if (TimeValueMapHandle_.isValid()) {
    timeLowerBound_ = fwitem->getConfig()->value<double>("TimeLowerBound(ns)");
    timeUpperBound_ = fwitem->getConfig()->value<double>("TimeUpperBound(ns)");
    if (timeLowerBound_ > timeUpperBound_) {
      edm::LogWarning("InvalidParameters")
          << "lower time bound is larger than upper time bound. Maybe opposite is desired?";
    }
  } else {
    edm::LogWarning("DataNotFound|InvalidData") << "couldn't locate 'timeLayerCluster' ValueMap in root file.";
  }

  if (!layerClustersHandle_.isValid()) {
    edm::LogWarning("DataNotFound|InvalidData") << "couldn't locate 'timeLayerCluster' ValueMap in root file.";
  }

  layer_ = fwitem->getConfig()->value<long>("Layer");
  saturation_energy_ = fwitem->getConfig()->value<double>("EnergyCutOff");
  heatmap_ = fwitem->getConfig()->value<bool>("Heatmap");
  z_plus_ = fwitem->getConfig()->value<bool>("Z+");
  z_minus_ = fwitem->getConfig()->value<bool>("Z-");
  enableTimeFilter_ = fwitem->getConfig()->value<bool>("EnableTimeFilter");
  enableSeedLines_ = fwitem->getConfig()->value<bool>("EnableSeedLines");
  enablePositionLines_ = fwitem->getConfig()->value<bool>("EnablePositionLines");
  enableEdges_ = fwitem->getConfig()->value<bool>("EnableEdges");

  FWHeatmapProxyBuilderTemplate::Build();
}
  
void FWTracksterHitsProxyBuilder::BuildItem(const ticl::Trackster &iData,
                                        int iIndex,
                                        REveElement *oItemHolder,
                                        const REveViewContext *) {
  if (enableTimeFilter_ && TimeValueMapHandle_.isValid()) {
    const float time = TimeValueMapHandle_->get(iIndex).first;
    if (time < timeLowerBound_ || time > timeUpperBound_)
      return;
  }

  const ticl::Trackster &trackster = iData;
  const size_t N = trackster.vertices().size();
  const std::vector<reco::CaloCluster> &layerClusters = *layerClustersHandle_;

  Color_t itemColor = Collection()->GetDataItem(iIndex)->GetMainColor();
  REveBoxSet *hex_boxset = new REveBoxSet();
  if (!heatmap_) {
    hex_boxset->UseSingleColor();
    hex_boxset->SetMainColor(itemColor);
    hex_boxset->SetMainColorPtr(new Color_t);
  }
  hex_boxset->SetPickable(true);
  hex_boxset->Reset(REveBoxSet::kBT_Hex, true, 64);

  REveBoxSet *boxset = new REveBoxSet();
  if (!heatmap_) {
    boxset->UseSingleColor();
    boxset->SetMainColor(itemColor);
    boxset->SetMainColorPtr(new Color_t);
  }
  boxset->SetPickable(true);
  boxset->Reset(REveBoxSet::kBT_FreeBox, true, 64);

  REveStraightLineSet *seed_marker = nullptr;
  if (enableSeedLines_) {
    seed_marker = new REveStraightLineSet("seeds");
    seed_marker->SetLineWidth(2);
    seed_marker->SetLineColor(kOrange + 10);
    seed_marker->GetLinePlex().Reset(sizeof(REveStraightLineSet::Line_t), 2 * N);
  }

  REveStraightLineSet *position_marker = nullptr;
  if (enablePositionLines_) {
    position_marker = new REveStraightLineSet("positions");
    position_marker->SetLineWidth(2);
    position_marker->SetLineColor(kOrange);
    position_marker->GetLinePlex().Reset(sizeof(REveStraightLineSet::Line_t), 2 * N);
  }

  auto geom = fireworks::Context::getInstance()->getGeom();
  for (size_t i = 0; i < N; ++i) {
    const reco::CaloCluster layerCluster = layerClusters[trackster.vertices(i)];
    std::vector<std::pair<DetId, float>> clusterDetIds = layerCluster.hitsAndFractions();

    for (std::vector<std::pair<DetId, float>>::iterator it = clusterDetIds.begin(), itEnd = clusterDetIds.end();
         it != itEnd;
         ++it) {
      const float *corners = geom->getCorners(it->first);
      if (corners == nullptr)
        continue;

      if (heatmap_ && m_hitmap.find(it->first) == m_hitmap.end())
        continue;

      const float *parameters = geom->getParameters(it->first);
      const float *shapes = geom->getShapePars(it->first);

      if (parameters == nullptr || shapes == nullptr)
        continue;

      const int total_points = parameters[0];
      const int layer = parameters[1];
      const int zside = parameters[2];
      const bool isSilicon = parameters[3];

      // discard everything that's not at the side that we are intersted in
      auto const z_selection_is_on = z_plus_ ^ z_minus_;
      auto const z_plus_selection_ok = z_plus_ && (zside == 1);
      auto const z_minus_selection_ok = z_minus_ && (zside == -1);
      if (!z_minus_ && !z_plus_)
        break;
      if (z_selection_is_on && !(z_plus_selection_ok || z_minus_selection_ok))
        break;

      if (layer_ > 0 && layer != layer_)
        break;

      // seed and cluster position
      if (layerCluster.seed().rawId() == it->first.rawId()) {
        const float crossScale = 0.2f + fmin(layerCluster.energy(), 5.0f);
        if (enableSeedLines_) {
          // center of RecHit
          const float center[3] = {corners[total_points * 3 + 0],
                                   corners[total_points * 3 + 1],
                                   corners[total_points * 3 + 2] + shapes[3] * 0.5f};

          // draw 3D cross
          seed_marker->AddLine(
              center[0] - crossScale, center[1], center[2], center[0] + crossScale, center[1], center[2]);
          seed_marker->AddLine(
              center[0], center[1] - crossScale, center[2], center[0], center[1] + crossScale, center[2]);
        }

        if (enablePositionLines_) {
          auto const &pos = layerCluster.position();
          const float position_crossScale = crossScale * 0.5;
          position_marker->AddLine(
              pos.x() - position_crossScale, pos.y(), pos.z(), pos.x() + position_crossScale, pos.y(), pos.z());
          position_marker->AddLine(
              pos.x(), pos.y() - position_crossScale, pos.z(), pos.x(), pos.y() + position_crossScale, pos.z());
        }
      }

    auto fwitem = dynamic_cast<FWWebEventItem *>(Collection());
      const float energy =
          fmin((fwitem->getConfig()->value<bool>("Cluster(0)/RecHit(1)") ? m_hitmap.at(it->first)->energy()
                                                                         : layerCluster.energy()) /
                   saturation_energy_,
               1.0f);
      const uint8_t colorFactor = FWHGCAL_GRADIENT_STEPS * energy;
      //auto transparency = item()->modelInfo(iIndex).displayProperties().transparency();
     // UChar_t alpha = (255 * (100 - transparency)) / 100;
      UChar_t alpha = 255; // AMT

      // Scintillator
      if (!isSilicon) {
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
        if (heatmap_) {
          energy
              ? boxset->DigitColor(fwhgcal::gradient[0][colorFactor], fwhgcal::gradient[1][colorFactor], fwhgcal::gradient[2][colorFactor], alpha)
              : boxset->DigitColor(64, 64, 64, alpha);
        }
      }
      // Silicon
      else {
        constexpr int offset = 9;

        float centerX = (corners[6] + corners[6 + offset]) / 2;
        float centerY = (corners[7] + corners[7 + offset]) / 2;
        float radius = fabs(corners[6] - corners[6 + offset]) / 2;
        hex_boxset->AddHex(REveVector(centerX, centerY, corners[2]), radius, 90.0, shapes[3]);
        if (heatmap_) {
          energy ? hex_boxset->DigitColor(
                       fwhgcal::gradient[0][colorFactor], fwhgcal::gradient[1][colorFactor], fwhgcal::gradient[2][colorFactor], alpha)
                 : hex_boxset->DigitColor(64, 64, 64, alpha);
        } else {
          hex_boxset->CSCApplyMainColorToMatchingChildren();
          hex_boxset->CSCApplyMainTransparencyToMatchingChildren();
          hex_boxset->SetMainColor(Collection()->GetMainColor());
          hex_boxset->SetMainTransparency(Collection()->GetMainTransparency());
        }
      }
    }  // End of loop over rechits of a single layercluster
  }    // End loop over the layerclusters of the trackster

  hex_boxset->RefitPlex();
  boxset->RefitPlex();
  SetupAddElement(hex_boxset, oItemHolder);
  SetupAddElement(boxset, oItemHolder);

  if (enableSeedLines_)
    oItemHolder->AddElement(seed_marker);

  if (enablePositionLines_)
    oItemHolder->AddElement(position_marker);

  if (enableEdges_) {
    auto &edges = trackster.edges();

    REveStraightLineSet *adjacent_marker = new REveStraightLineSet("adj_edges");
    adjacent_marker->SetLineWidth(2);
    adjacent_marker->SetLineColor(kYellow);
    adjacent_marker->GetLinePlex().Reset(sizeof(REveStraightLineSet::Line_t), edges.size());

    REveStraightLineSet *non_adjacent_marker = new REveStraightLineSet("non_adj_edges");
    non_adjacent_marker->SetLineWidth(2);
    non_adjacent_marker->SetLineColor(kRed);
    non_adjacent_marker->GetLinePlex().Reset(sizeof(REveStraightLineSet::Line_t), edges.size());

    for (auto edge : edges) {
      auto doublet = std::make_pair(layerClusters[edge[0]], layerClusters[edge[1]]);

      int layerIn = geom->getParameters(doublet.first.seed())[1];
      int layerOut = geom->getParameters(doublet.second.seed())[1];

      const bool isAdjacent = std::abs(layerOut - layerIn) == 1;

      // draw 3D cross
      if (layer_ == 0 || std::abs(layerIn - layer_) == 0 || std::abs(layerOut - layer_) == 0) {
        if (isAdjacent)
          adjacent_marker->AddLine(doublet.first.x(),
                                   doublet.first.y(),
                                   doublet.first.z(),
                                   doublet.second.x(),
                                   doublet.second.y(),
                                   doublet.second.z());
        else
          non_adjacent_marker->AddLine(doublet.first.x(),
                                       doublet.first.y(),
                                       doublet.first.z(),
                                       doublet.second.x(),
                                       doublet.second.y(),
                                       doublet.second.z());
      }
    }  // End of loop over all edges of the trackster
    oItemHolder->AddElement(adjacent_marker);
    oItemHolder->AddElement(non_adjacent_marker);
  }
}

REGISTER_FW2PROXYBUILDER(FWTracksterHitsProxyBuilder, ticl::Trackster, "Trackster hits");
