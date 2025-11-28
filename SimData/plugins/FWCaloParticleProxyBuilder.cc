#include "FireworksWeb/Core/interface/FWProxyBuilderFactory.h"
#include "FireworksWeb/Calo/interface/FWHeatmapProxyBuilderTemplate.h"

#include "ROOT/REvePointSet.hxx"
#include "ROOT/REveBoxSet.hxx"
#include "ROOT/REveViewContext.hxx"
#include "ROOT/REveDataSimpleProxyBuilderTemplate.hxx"

#include "FireworksWeb/Core/interface/FWGeometry.h"
#include "FireworksWeb/Core/interface/fwLog.h"
#include "FireworksWeb/Core/interface/Context.h"


#include "SimDataFormats/CaloAnalysis/interface/CaloParticle.h"
#include "SimDataFormats/CaloAnalysis/interface/CaloParticleFwd.h"
#include "SimDataFormats/CaloAnalysis/interface/SimCluster.h"
#include "DataFormats/HGCRecHit/interface/HGCRecHitCollections.h"

using namespace ROOT::Experimental;


class FWCaloParticleProxyBuilder : public FWHeatmapProxyBuilderTemplate<CaloParticle>
{
public:
    REGISTER_FWPB_METHODS();

    using FWHeatmapProxyBuilderTemplate<CaloParticle>::BuildItem;
    virtual void BuildItem(const CaloParticle& iData, int /*idx*/, REveElement* iItemHolder, const REveViewContext* vc) override;
};

void FWCaloParticleProxyBuilder::BuildItem(const CaloParticle &iData, int idx, ROOT::Experimental::REveElement *oItemHolder, const ROOT::Experimental::REveViewContext *context)
{
  auto fwItem = dynamic_cast<FWWebEventItem *>(Collection());
  const long layer = fwItem->getConfig()->value<long>("Layer");
  const double saturation_energy = fwItem->getConfig()->value<double>("EnergyCutOff");
  const bool z_plus = fwItem->getConfig()->value<bool>("Z+");
  const bool z_minus = fwItem->getConfig()->value<bool>("Z-");


  bool heatmap = fwItem->getConfig()->value<bool>("Heatmap");
  if (heatmap && m_hitmap.empty()) {
    fwLog(fwlog::kError) << "FWCaloParticleProxyBuilder::BuildItem: Hit association map is empty. Disable hitmap!";
    heatmap = false;
  }


  Color_t itemColor = Collection()->GetDataItem(idx)->GetMainColor();

  bool h_hex(false);
  REveBoxSet *hex_boxset = new REveBoxSet();
  if (!heatmap) {
    hex_boxset->UseSingleColor();
    hex_boxset->SetMainColorPtr(new Color_t);
    hex_boxset->SetMainColor(itemColor);
  }
  hex_boxset->SetPickable(true);
  hex_boxset->Reset(REveBoxSet::kBT_InstancedScaledRotated, true, 64);
  hex_boxset->SetAntiFlick(true);

  bool h_box(false);
  REveBoxSet *boxset = new REveBoxSet();
  if (!heatmap) {
    boxset->UseSingleColor();
    boxset->SetMainColor(itemColor);
    boxset->SetMainColorPtr(new Color_t);
  }
  boxset->SetPickable(true);
  boxset->Reset(REveBoxSet::kBT_FreeBox, true, 64);
  boxset->SetAntiFlick(true);

  auto geom = fireworks::Context::getInstance()->getGeom();

  for (const auto &c : iData.simClusters()) {
    for (const auto &it : (*c).hits_and_fractions()) {
      if (heatmap && m_hitmap.find(it.first) == m_hitmap.end())
        continue;

      const bool z = (it.first >> 25) & 0x1;

      // discard everything thats not at the side that we are intersted in
      if (((z_plus & z_minus) != 1) && (((z_plus | z_minus) == 0) || !(z == z_minus || z == !z_plus)))
        continue;

      const float *corners = geom->getCorners(it.first);
      const float *parameters = geom->getParameters(it.first);
      const float *shapes = geom->getShapePars(it.first);

      if (corners == nullptr || parameters == nullptr || shapes == nullptr) {
        continue;
      }

      const int total_points = parameters[0];
      const bool isScintillator = (total_points == 4);
      const uint8_t type = ((it.first >> 28) & 0xF);

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

        if (ll != ((it.first >> (isScintillator ? 17 : 20)) & 0x1F))
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
          const uint8_t colorFactor = FWHGCAL_GRADIENT_STEPS * (fmin(m_hitmap.at(it.first)->energy() / saturation_energy, 1.0f));
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
        // hex_boxset->AddHex(REveVector(centerX, centerY, corners[2]), radius, shapes[2], shapes[3]);
        /* The AddHex function angle seems to make a rotation around (0,0,0) (to be verified).  (REveBoxSet::AddHex(const REveVector& pos, Float_t r, Float_t angle, Float_t depth) https://root.cern/doc/v638/REveBoxSet_8cxx_source.html#l00228 )
        While we want to make a rotation around the center of the hexagon.
        For now set the angle to 0 (the hexagon will not have the correct rotation but its center will be in the correct location).
        TODO do the rotation properly.
        */
        hex_boxset->AddHex(REveVector(centerX, centerY, corners[2]), radius, 0., shapes[3]); 
        // Needs to be properly computed
        if (heatmap) {
          const uint8_t colorFactor = FWHGCAL_GRADIENT_STEPS * (fmin(m_hitmap.at(it.first)->energy() / saturation_energy, 1.0f));
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

REGISTER_FW2PROXYBUILDER(FWCaloParticleProxyBuilder, CaloParticle, "CaloParticle");
