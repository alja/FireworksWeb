#ifndef FireworksWeb_Core_FWViewEnergyScale_h
#define FireworksWeb_Core_FWViewEnergyScale_h

#include <string>
#include <sigc++/sigc++.h>

class FWViewEnergyScale {

public:
  enum EScaleMode { kFixedScale, kAutoScale, kCombinedScale, kNone };
  FWViewEnergyScale(std::string name);
  virtual ~FWViewEnergyScale();

  void updateScaleFactors(float iMaxVal);

  float getScaleFactor3D() const { return m_scaleFactor3D; }
  float getScaleFactorLego() const { return m_scaleFactorLego; }

  bool getPlotEt() const { return m_plotEt; }

  void scaleParameterChanged() const;

  sigc::signal<void> parameterChanged_;

  const std::string& name() const { return m_name; }

 // void setFrom(const FWConfiguration&) override;
 // void SetFromCmsShowCommonConfig(long mode, float convert, float maxH, bool et);

protected:
  EScaleMode m_scaleMode;
  double m_fixedValToHeight;
  double m_maxTowerHeight;
  double m_plotEt;

private:
  FWViewEnergyScale(const FWViewEnergyScale&) = delete;                   // stop default
  const FWViewEnergyScale& operator=(const FWViewEnergyScale&) = delete;  // stop default

  float calculateScaleFactor(float iMaxVal, bool isLego) const;

  const std::string m_name;

  // cached
  float m_scaleFactor3D;
  float m_scaleFactorLego;
};

#endif
