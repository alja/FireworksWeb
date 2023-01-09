#include "FireworksWeb/Core/interface/FWProxyBuilderFactory.h"
#include "FireworksWeb/Calo/interface/FWHeatmapProxyBuilderTemplate.h"

#include "ROOT/REvePointSet.hxx"
#include "ROOT/REveViewContext.hxx"
#include "ROOT/REveStraightLineSet.hxx"
#include "ROOT/REveGeoShape.hxx"
#include "TGeoTube.h"

#include "FireworksWeb/Core/interface/FWGeometry.h"
#include "FireworksWeb/Core/interface/fwLog.h"
#include "FireworksWeb/Core/interface/Context.h"
#include "FireworksWeb/Core/interface/BuilderUtils.h"

#include "DataFormats/HGCalReco/interface/Trackster.h"
#include "DataFormats/CaloRecHit/interface/CaloCluster.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/DetId/interface/DetId.h"
#include "DataFormats/ForwardDetId/interface/HGCScintillatorDetId.h"
#include "DataFormats/ForwardDetId/interface/HGCSiliconDetId.h"
#include "DataFormats/FWLite/interface/Event.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"


using namespace ROOT::Experimental;
class FWTracksterLayersProxyBuilder : public FWHeatmapProxyBuilderTemplate<ticl::Trackster>
{
    public:
    REGISTER_FWPB_METHODS();

    using FWHeatmapProxyBuilderTemplate::SetCollection;
    void SetCollection(REveDataCollection* c) override {
        auto item = dynamic_cast<FWWebEventItem *>(c);
        item->getConfig()->assertParam("EnablePositionLines", false);
        item->getConfig()->assertParam("EnableEdges", false);
        item->getConfig()->assertParam("EnableTimeFilter", false);
        item->getConfig()->assertParam("TimeLowerBound(ns)", 0.01, 0.0, 75.0);
        item->getConfig()->assertParam("TimeUpperBound(ns)", 0.01, 0.0, 75.0);
        item->getConfig()->assertParam("DisplayMode", 0.0, 0.0, 5.0);
        item->getConfig()->assertParam("ProportionalityFactor", 1.0, 0.0, 1.0);
        FWHeatmapProxyBuilderTemplate::SetCollection(c);
    }

    using FWHeatmapProxyBuilderTemplate<ticl::Trackster>::Build;
    void Build() override;

    using FWHeatmapProxyBuilderTemplate<ticl::Trackster>::BuildItem;
    virtual void BuildItem(const ticl::Trackster &iData, int idx, ROOT::Experimental::REveElement *iItemHolder, const ROOT::Experimental::REveViewContext *context) override;

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
    bool enablePositionLines_;
    bool enableEdges_;
    double displayMode_;
    double proportionalityFactor_;
};

//------------------------------------------------------------
void FWTracksterLayersProxyBuilder::Build() {
  
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
  enablePositionLines_ = fwitem->getConfig()->value<bool>("EnablePositionLines");
  enableEdges_ = fwitem->getConfig()->value<bool>("EnableEdges");
  displayMode_ = fwitem->getConfig()->value<double>("DisplayMode");
  proportionalityFactor_ = fwitem->getConfig()->value<double>("ProportionalityFactor");


  FWHeatmapProxyBuilderTemplate::Build();
}

//------------------------------------------------------------
void FWTracksterLayersProxyBuilder::BuildItem(const ticl::Trackster &iData,
                                          int idx, ROOT::Experimental::REveElement *oItemHolder, 
                                          const ROOT::Experimental::REveViewContext *context)
{ 
  if (enableTimeFilter_ && TimeValueMapHandle_.isValid()) {
    const float time = TimeValueMapHandle_->get(idx).first;
    if (time < timeLowerBound_ || time > timeUpperBound_)
      return;
  }

  auto geom = fireworks::Context::getInstance()->getGeom();

  const ticl::Trackster &trackster = iData;
  const size_t N = trackster.vertices().size();
  const std::vector<reco::CaloCluster> &layerClusters = *layerClustersHandle_;
  REveStraightLineSet *position_marker = nullptr;

  if (enablePositionLines_) {
    position_marker = new REveStraightLineSet;
    position_marker->SetLineWidth(2);
    position_marker->SetLineColor(kWhite);
  }

  for (size_t i = 0; i < N; ++i) {
    const reco::CaloCluster layerCluster = layerClusters[trackster.vertices(i)];
    const math::XYZPoint &position = layerCluster.position();
    const size_t nHits = layerCluster.size();
    const double energy = layerCluster.energy();
    float radius = 0;
    auto detIdOnLayer = layerCluster.seed();

    const auto *parameters = geom->getParameters(detIdOnLayer);
    const int layer = parameters[1];
    const int zside = parameters[2];
    const bool isSilicon = parameters[3];

    auto const z_selection_is_on = z_plus_ ^ z_minus_;
    auto const z_plus_selection_ok = z_plus_ && (zside == 1);
    auto const z_minus_selection_ok = z_minus_ && (zside == -1);
    if (!z_minus_ && !z_plus_)
      continue;
    if (z_selection_is_on && !(z_plus_selection_ok || z_minus_selection_ok))
      continue;

    if (layer_ > 0 && (layer != layer_))
      continue;

    if (displayMode_ == 0) {
      radius = sqrt(nHits);
    } else if (displayMode_ == 1) {
      radius = nHits;
    } else if (displayMode_ == 2) {
      radius = energy;
    } else if (displayMode_ == 3) {
      radius = energy / nHits;
    } else if (displayMode_ == 4) {
      float area = 0;
      if (!isSilicon) {
        const bool isFine = (HGCScintillatorDetId(layerCluster.seed()).type() == 0);
        float dphi = (isFine) ? 1.0 * M_PI / 180. : 1.25 * M_PI / 180.;
        int ir = HGCScintillatorDetId(layerCluster.seed()).iradiusAbs();
        float dr = (isFine) ? (0.0484 * static_cast<float>(ir) + 2.1) : (0.075 * static_cast<float>(ir) + 2.0);
        float r = (isFine) ? (0.0239 * static_cast<float>(pow(ir, 2)) + 2.02 * static_cast<float>(ir) + 119.6)
                           : (0.0367 * static_cast<float>(pow(ir, 2)) + 1.7 * static_cast<float>(ir) + 90.7);
        area = r * dr * dphi;
      } else {
        const bool isFine = (HGCSiliconDetId(layerCluster.seed()).type() == 0);
        float side = (isFine) ? 0.465 : 0.698;
        area = pow(side, 2) * 3 * sqrt(3) / 2;
      }
      radius = sqrt(nHits * area) / M_PI;
    }

    auto *eveCircle = new REveGeoShape("Circle");
    auto tube = new TGeoTube(0., proportionalityFactor_ * radius, 0.1);
    eveCircle->SetShape(tube);
    eveCircle->InitMainTrans();
    eveCircle->RefMainTrans().Move3PF(position.x(), position.y(), position.z());
    SetupAddElement(eveCircle, oItemHolder);
    // Apply heatmap color coding **after** the call to setupAddElement, that will internally setup the color.
    if (heatmap_) {
      const float normalized_energy = fmin(energy / saturation_energy_, 1.0f);
      const uint8_t colorFactor = FWHGCAL_GRADIENT_STEPS * normalized_energy;
      eveCircle->SetFillColor(
          TColor::GetColor(fwhgcal::gradient[0][colorFactor], fwhgcal::gradient[1][colorFactor], fwhgcal::gradient[2][colorFactor]));
    } else {
      Color_t itemColor = Collection()->GetDataItem(idx)->GetMainColor();
      eveCircle->SetMainColor(itemColor);
      eveCircle->SetMainTransparency(Collection()->GetMainTransparency());
    }

    // seed and cluster position
    const float crossScale = 1.0f + fmin(energy, 5.0f);
    if (enablePositionLines_) {
      auto const &pos = layerCluster.position();
      const float position_crossScale = crossScale * 0.5;
      position_marker->AddLine(
          pos.x() - position_crossScale, pos.y(), pos.z(), pos.x() + position_crossScale, pos.y(), pos.z());
      position_marker->AddLine(
          pos.x(), pos.y() - position_crossScale, pos.z(), pos.x(), pos.y() + position_crossScale, pos.z());
    }
  }

  if (enablePositionLines_)
    oItemHolder->AddElement(position_marker);

  if (enableEdges_) {
    auto &edges = trackster.edges();

    REveStraightLineSet *adjacent_marker = new REveStraightLineSet;
    adjacent_marker->SetLineWidth(2);
    adjacent_marker->SetLineColor(kYellow);

    REveStraightLineSet *non_adjacent_marker = new REveStraightLineSet;
    non_adjacent_marker->SetLineWidth(2);
    non_adjacent_marker->SetLineColor(kRed);

    for (auto edge : edges) {
      auto doublet = std::make_pair(layerClusters[edge[0]], layerClusters[edge[1]]);

      int layerIn  = geom->getParameters(doublet.first.seed())[1];
      int layerOut = geom->getParameters(doublet.second.seed())[1];

      const bool isAdjacent = std::abs(layerOut - layerIn) == 1;

      // draw 3D cross
      if (layer_ == 0 || fabs(layerIn - layer_) == 0 || fabs(layerOut - layer_) == 0) {
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
    }
    SetupAddElement(adjacent_marker, oItemHolder);
    SetupAddElement(non_adjacent_marker, oItemHolder);
  }
}

REGISTER_FW2PROXYBUILDER(FWTracksterLayersProxyBuilder, ticl::Trackster, "Trackster layers");
