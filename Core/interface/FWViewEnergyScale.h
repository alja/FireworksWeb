#ifndef FireworksWeb_Core_FWViewEnergyScale_h
#define FireworksWeb_Core_FWViewEnergyScale_h

#include <string>
#include <sigc++/sigc++.h>
#include "FireworksWeb/Core/interface/FWConfigurableParameterizable.h"


#include "FireworksWeb/Core/interface/FWDoubleParameter.h"
#include "FireworksWeb/Core/interface/FWBoolParameter.h"
#include "FireworksWeb/Core/interface/FWLongParameter.h"
#include "FireworksWeb/Core/interface/FWEnumParameter.h"


#include "ROOT/REveElement.hxx"

class FWViewEnergyScale : public ROOT::Experimental::REveElement,
                          public FWConfigurableParameterizable
{
public:
  enum EScaleMode
  {
    kFixedScale,
    kAutoScale,
    kCombinedScale,
    kNone
  };
  FWViewEnergyScale(std::string name, int version = 7);
  virtual ~FWViewEnergyScale();

  void updateScaleFactors(float iMaxVal);

  float getScaleFactor3D() const { return m_scaleFactor3D; }
  float getScaleFactorLego() const { return m_scaleFactorLego; }

  bool getPlotEt() const { return m_plotEt.value(); }

  void scaleParameterChanged() const;

  sigc::signal<void()> parameterChanged_;

  const std::string &name() const { return m_name; }

  int WriteCoreJson(nlohmann::json &j, int rnr_offset) override;

  void ScaleChanged(const char *arg);
  // void setFrom(const FWConfiguration&) override;
  // void SetFromCmsShowCommonConfig(long mode, float convert, float maxH, bool et);

   void setDrawBarrel(bool);

  // ---------- const member functions ---------------------
  void addTo(FWConfiguration&) const override;

  // ---------- static member functions --------------------

  // ---------- member functions ---------------------------
  void setFrom(const FWConfiguration&) override;


protected:
  FWEnumParameter m_scaleMode;
  FWDoubleParameter m_fixedValToHeight;
  FWDoubleParameter m_maxTowerHeight;
  FWBoolParameter m_plotEt;

  FWBoolParameter m_drawBarrel;

/*
  EScaleMode m_scaleMode;
  double m_fixedValToHeight;
  double m_maxTowerHeight;
  bool m_plotEt;
  */
 

private:
  FWViewEnergyScale(const FWViewEnergyScale &) = delete;                  // stop default
  const FWViewEnergyScale &operator=(const FWViewEnergyScale &) = delete; // stop default

  float calculateScaleFactor(float iMaxVal, bool isLego) const;

  const std::string m_name;

  // cached
  float m_scaleFactor3D;
  float m_scaleFactorLego;
};

#endif
