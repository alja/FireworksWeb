#include "FireworksWeb/Core/interface/FWProxyBuilderFactory.h"
#include "FireworksWeb/Calo/interface/FWHeatmapProxyBuilderTemplate.h"

#include "ROOT/REvePointSet.hxx"
#include "ROOT/REveBoxSet.hxx"
#include "ROOT/REveRGBAPalette.hxx"
#include "ROOT/REveViewContext.hxx"
#include "ROOT/REveStraightLineSet.hxx"
#include "ROOT/REveDataSimpleProxyBuilderTemplate.hxx"

#include "FireworksWeb/Core/interface/FWGeometry.h"
#include "FireworksWeb/Core/interface/fwLog.h"
#include "FireworksWeb/Core/interface/Context.h"
#include "FireworksWeb/Core/interface/BuilderUtils.h"

#include "DataFormats/ParticleFlowReco/interface/HGCalMultiCluster.h"
#include "DataFormats/HGCRecHit/interface/HGCRecHitCollections.h"


using namespace ROOT::Experimental;
class FWHGCalMultiClusterProxyBuilder : public FWHeatmapProxyBuilderTemplate<reco::HGCalMultiCluster>
{
    public:
    REGISTER_FWPB_METHODS();

    using FWHeatmapProxyBuilderTemplate<reco::HGCalMultiCluster>::BuildItem;
    virtual void BuildItem(const reco::HGCalMultiCluster &iData, int idx, ROOT::Experimental::REveElement *iItemHolder, const ROOT::Experimental::REveViewContext *context) override;

};

void FWHGCalMultiClusterProxyBuilder::BuildItem(const reco::HGCalMultiCluster &iData,
                                          int idx, ROOT::Experimental::REveElement *oItemHolder, 
                                          const ROOT::Experimental::REveViewContext *context)
{ 
  auto fwitem = dynamic_cast<FWWebEventItem *>(Collection());
  const long layer = fwitem->getConfig()->value<long>("Layer");
  const double saturation_energy = fwitem->getConfig()->value<double>("EnergyCutOff");
  const bool heatmap = fwitem->getConfig()->value<bool>("Heatmap");
  const bool z_plus = fwitem->getConfig()->value<bool>("Z+");
  const bool z_minus = fwitem->getConfig()->value<bool>("Z-");

  const auto &clusters = iData.clusters();
  Color_t itemColor = Collection()->GetDataItem(idx)->GetMainColor();

  bool h_hex(false);
  REveBoxSet *hex_boxset = new REveBoxSet("Silicon");
  if (!heatmap){
    hex_boxset->UseSingleColor();
    hex_boxset->SetMainColorPtr(new Color_t);
    hex_boxset->SetMainColor(itemColor);
  }
  hex_boxset->SetPickable(true);
  hex_boxset->Reset(REveBoxSet::kBT_Hex, true, 64);
  hex_boxset->SetAntiFlick(kTRUE);

  bool h_box(false);
  REveBoxSet *boxset = new REveBoxSet("Scintillator");
  if (!heatmap){
    boxset->UseSingleColor();
    boxset->SetMainColorPtr(new Color_t);
    boxset->SetMainColor(itemColor);
  }
  boxset->SetPickable(true);
  boxset->Reset(REveBoxSet::kBT_FreeBox, true, 64);
  boxset->SetAntiFlick(kTRUE);

  auto geom = fireworks::Context::getInstance()->getGeom();

  for (const auto &c : clusters) {
    std::vector<std::pair<DetId, float>> clusterDetIds = c->hitsAndFractions();

    for (std::vector<std::pair<DetId, float>>::iterator it = clusterDetIds.begin(), itEnd = clusterDetIds.end();
         it != itEnd;
         ++it) {
      if (heatmap && m_hitmap.find(it->first) == m_hitmap.end())
        continue;

      const bool z = (it->first >> 25) & 0x1;

      // discard everything thats not at the side that we are intersted in
      if (((z_plus & z_minus) != 1) && (((z_plus | z_minus) == 0) || !(z == z_minus || z == !z_plus)))
        continue;

      const float *corners = geom->getCorners(it->first);
      const float *parameters = geom->getParameters(it->first);
      const float *shapes = geom->getShapePars(it->first);

      if (corners == nullptr || parameters == nullptr || shapes == nullptr) {
        continue;
      }

      const int total_points = parameters[0];
      const bool isScintillator = (total_points == 4);
      const uint8_t type = ((it->first >> 28) & 0xF);

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
          const uint8_t colorFactor = FWHGCAL_GRADIENT_STEPS * (fmin(m_hitmap[it->first]->energy() / saturation_energy, 1.0f));
          boxset->DigitColor(fwhgcal::gradient[0][colorFactor], fwhgcal::gradient[1][colorFactor], fwhgcal::gradient[2][colorFactor]);
        }

        h_box = true;
      }
      // Silicon
      else {
        const int offset = 9;

        float centerX = (corners[6] + corners[6 + offset]) / 2;
        float centerY = (corners[7] + corners[7 + offset]) / 2;
        float radius = fabs(corners[6] - corners[6 + offset]) / 2;
        hex_boxset->AddHex(REveVector(centerX, centerY, corners[2]), radius, 90.0, shapes[3]);
        if (heatmap) {
          const uint8_t colorFactor = FWHGCAL_GRADIENT_STEPS * (fmin(m_hitmap[it->first]->energy() / saturation_energy, 1.0f));
          hex_boxset->DigitColor(fwhgcal::gradient[0][colorFactor], fwhgcal::gradient[1][colorFactor], fwhgcal::gradient[2][colorFactor]);
        }

        h_hex = true;
      }
    }
  }

  if (h_hex) {
    hex_boxset->RefitPlex();
    SetupAddElement(hex_boxset, oItemHolder);
  }

  if (h_box) {
    boxset->RefitPlex();
    SetupAddElement(boxset, oItemHolder);
  }
}


REGISTER_FW2PROXYBUILDER(FWHGCalMultiClusterProxyBuilder, reco::HGCalMultiCluster, "HGCal MultiCluster");
