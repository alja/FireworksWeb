#include "FireworksWeb/Calo/interface/FWCaloRecHitDigitSetProxyBuilder.h"
#include "FireworksWeb/Core/interface/BuilderUtils.h"
#include "FireworksWeb/Core/interface/FWViewEnergyScale.h"
#include "FireworksWeb/Core/interface/Context.h"
#include "FireworksWeb/Core/interface/FWGeometry.h"

#include "ROOT/REveBoxSet.hxx"

#include "DataFormats/CaloRecHit/interface/CaloRecHit.h"
#include <assert.h>

using namespace ROOT::Experimental;

FWCaloRecHitDigitSetProxyBuilder::FWCaloRecHitDigitSetProxyBuilder()
    : m_invertBox(false), m_ignoreGeoShapeSize(false), m_enlarge(1.0) {}


//______________________________________________________________________________

void FWCaloRecHitDigitSetProxyBuilder::viewContextBoxScale(
    const float* corners, float scale, bool plotEt, std::vector<float>& scaledCorners, const CaloRecHit*) {
  scale *= m_enlarge;

  REveVector v; 
  if (m_ignoreGeoShapeSize) {
    // Same functionality as fireworks::energyTower3DCorners()

    for (int i = 0; i < 24; ++i)
      scaledCorners[i] = corners[i];
    // Coordinates of a front face scaled
    if (m_invertBox) {
      // We know, that an ES rechit geometry in -Z needs correction.
      // The back face is actually its front face.
      for (unsigned int i = 0; i < 12; i += 3) {
        v.Set(corners[i] - corners[i + 12], corners[i + 1] - corners[i + 13], corners[i + 2] - corners[i + 14]);
        v.Normalize();
        v *= scale;

        scaledCorners[i] = corners[i] + v.fX;
        scaledCorners[i + 1] = corners[i + 1] + v.fY;
        scaledCorners[i + 2] = corners[i + 2] + v.fZ;
      }
    } else {
      for (unsigned int i = 0; i < 12; i += 3) {
        v.Set(corners[i + 12] - corners[i], corners[i + 13] - corners[i + 1], corners[i + 14] - corners[i + 2]);
        v.Normalize();
        v *= scale;

        scaledCorners[i] = corners[i + 12];
        scaledCorners[i + 1] = corners[i + 13];
        scaledCorners[i + 2] = corners[i + 14];

        scaledCorners[i + 12] = corners[i + 12] + v.fX;
        scaledCorners[i + 13] = corners[i + 13] + v.fY;
        scaledCorners[i + 14] = corners[i + 14] + v.fZ;
      }
    }
  } else {
    // Same functionality as fireworks::energyScaledBox3DCorners().

    v.Set(0.f, 0.f, 0.f);
    for (unsigned int i = 0; i < 24; i += 3) {
      v[0] += corners[i];
      v[1] += corners[i + 1];
      v[2] += corners[i + 2];
    }
    v *= 1.f / 8.f;

    if (plotEt) {
      scale *= v.Perp() / v.Mag();
    }

    // Coordinates for a scaled version of the original box
    for (unsigned int i = 0; i < 24; i += 3) {
      scaledCorners[i] = v[0] + (corners[i] - v[0]) * scale;
      scaledCorners[i + 1] = v[1] + (corners[i + 1] - v[1]) * scale;
      scaledCorners[i + 2] = v[2] + (corners[i + 2] - v[2]) * scale;
    }

    if (m_invertBox)
      fireworks::invertBox(scaledCorners);
  }
}
//_____________________________________________________________________________

float FWCaloRecHitDigitSetProxyBuilder::scaleFactor(const FWViewEnergyScale* ec) {
  // printf("scale face %f \n", vc->getEnergyScale()->getScaleFactor3D());
  return ec->getScaleFactor3D() / 50;
}


//______________________________________________________________________________
void  FWCaloRecHitDigitSetProxyBuilder::ScaleProduct(ROOT::Experimental::REveElement *parent, const std::string &vtype)
{
  size_t size = Collection()->GetNItems();
  if (!size)
    return;

  auto energyScale = fireworks::Context::getInstance()->energyScale();
  std::vector<float> scaledCorners(24);
  float scale = scaleFactor(energyScale);

  assert(parent->NumChildren() == 1);

  for (int index = 0; index < static_cast<int>(size); ++index) {
    const CaloRecHit* hit = (const CaloRecHit*)Collection()->GetDataPtr(index);
    const float* corners = fireworks::Context::getInstance()->getGeom()->getCorners(hit->detid());
    if (corners == nullptr)
      continue;


    viewContextBoxScale(corners, hit->energy() * scale, energyScale->getPlotEt(), scaledCorners, hit);
    
    REveBoxSet::BFreeBox_t* b = (REveBoxSet::BFreeBox_t*)m_boxSet->GetPlex()->Atom(index);
    memcpy(b->fVertices, &scaledCorners[0], sizeof(b->fVertices));
  }
  m_boxSet->StampObjProps();
}

//______________________________________________________________________________

void FWCaloRecHitDigitSetProxyBuilder::BuildProduct(const ROOT::Experimental::REveDataCollection* collection, ROOT::Experimental::REveElement* product, const ROOT::Experimental::REveViewContext*)
{
    size_t size = Collection()->GetNItems();

    if (!product->HasChildren())
       m_boxSet = addBoxSetToProduct(product);

    m_boxSet->Reset(REveBoxSet::kBT_FreeBox, true, 256);

    // m_ignoreGeoShapeSize = item()->getConfig()->value<bool>("IgnoreShapeSize");
    // m_enlarge = item()->getConfig()->value<double>("Enlarge");
    m_enlarge = 1; //AMT

    std::vector<float> scaledCorners(24);
    auto energyScale = fireworks::Context::getInstance()->energyScale();
    float scale = scaleFactor(energyScale);

    REveVector v;
    for (int index = 0; index < static_cast<int>(size); ++index)
    {
        const CaloRecHit *hit = (const CaloRecHit*)Collection()->GetDataPtr(index);

        const float *corners = fireworks::Context::getInstance()->getGeom()->getCorners(hit->detid());
        if (corners)
        {
            v.Set(0.f, 0.f, 0.f);
            for (unsigned int i = 0; i < 24; i += 3)
            {
                v[0] += corners[i];
                v[1] += corners[i + 1];
                v[2] += corners[i + 2];
            }
            v.Normalize();
            fireworks::Context::getInstance()->voteMaxEtAndEnergy(v.Perp() * hit->energy(), hit->energy());
            viewContextBoxScale(corners, hit->energy() * scale, energyScale->getPlotEt(), scaledCorners, hit);
        }

        addBox(m_boxSet, &scaledCorners[0], Collection()->GetDataItem(index));
    }
}
