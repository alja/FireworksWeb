#ifndef FireworksWeb_Calo_FWCaloRecHitDigitSetProxyBuilder_h
#define FireworksWeb_Calo_FWCaloRecHitDigitSetProxyBuilder_h

#include "FireworksWeb/Core/interface/FWDigitSetProxyBuilder.h"

class CaloRecHit;
class FWViewEnergyScale;

class FWCaloRecHitDigitSetProxyBuilder : public FWDigitSetProxyBuilder {
public:
  FWCaloRecHitDigitSetProxyBuilder();
  ~FWCaloRecHitDigitSetProxyBuilder(void) override {}

  void ScaleProduct(ROOT::Experimental::REveElement *parent, const std::string &vtype) override;

 using REveDataProxyBuilderBase::BuildProduct;
   void BuildProduct(const ROOT::Experimental::REveDataCollection* collection, ROOT::Experimental::REveElement* product, const ROOT::Experimental::REveViewContext*)override;

 // virtual float scaleFactor(const FWViewContext* vc);
  virtual void invertBox(bool x) { m_invertBox = x; }
  virtual void viewContextBoxScale(const float* corners, float scale, bool plotEt, std::vector<float>& scaledCorners, const CaloRecHit*);

protected:
  float scaleFactor(const FWViewEnergyScale* ec);

private:
  FWCaloRecHitDigitSetProxyBuilder(const FWCaloRecHitDigitSetProxyBuilder&) = delete;
  const FWCaloRecHitDigitSetProxyBuilder& operator=(const FWCaloRecHitDigitSetProxyBuilder&) = delete;

  bool m_invertBox;
  bool m_ignoreGeoShapeSize;
  double m_enlarge;
};

#endif
