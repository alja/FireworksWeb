
#include <iostream>

#include "Rtypes.h"
#include "TMath.h"
#include "FireworksWeb/Core/interface/FWEveView.h"
#include "FireworksWeb/Core/interface/FWViewEnergyScale.h"
#include "FireworksWeb/Core/interface/Context.h"
#include "FireworksWeb/Core/interface/FWParameterBase.h"
#include "FireworksWeb/Core/interface/FWConfiguration.h"
 
#include "TBase64.h"

#include "nlohmann/json.hpp"
FWViewEnergyScale::FWViewEnergyScale(std::string name, int version):

      m_scaleMode(this, "ScaleMode", 1l, 1l, 2l),
      m_fixedValToHeight(this, "EnergyToLength [GeV/m]", 50.0, 1.0, 1000.0),
      m_maxTowerHeight(this, "MaximumLength [m]", 3.0, 0.01, 30.0),
      m_plotEt(this, "PlotEt", true),

      m_name(name),
      m_scaleFactor3D(1.f),
      m_scaleFactorLego(0.05f)
       {

      SetName("EnergyScale");
}

FWViewEnergyScale::~FWViewEnergyScale() {}

//________________________________________________________

void FWViewEnergyScale::scaleParameterChanged() const { parameterChanged_.emit(); }

float FWViewEnergyScale::calculateScaleFactor(float iMaxVal, bool isLego) const {
  // check if in combined mode
  int mode = m_scaleMode.value();
  if (mode == kCombinedScale) {
    mode = (m_maxTowerHeight.value() > 100 * iMaxVal / m_fixedValToHeight.value()) ? kFixedScale : kAutoScale;
    // printf("COMBINED  \n");
  }
  // get converison

  if (mode == kFixedScale) {
    //  printf("fixed mode %f \n",m_fixedValToHeight.);
    // apply default constructor height
    float length = isLego ? TMath::Pi() : 100;
    return length / m_fixedValToHeight.value();
  } else {
    float length = isLego ? TMath::Pi() : (100 * m_maxTowerHeight.value());
    // printf("[%d] length %f max %f  \n", isLego, length, iMaxVal);
    return length / iMaxVal;
  }
}

void FWViewEnergyScale::updateScaleFactors(float iMaxVal) {
  m_scaleFactor3D = calculateScaleFactor(iMaxVal, false);
  m_scaleFactorLego = calculateScaleFactor(iMaxVal, true);
}

int FWViewEnergyScale::WriteCoreJson(nlohmann::json &j, int rnr_offset)
{
  int ret = REveElement::WriteCoreJson(j, rnr_offset);

  j["plotEt"] = m_plotEt.value();
  j["mode"] = std::to_string(m_scaleMode.value());
  j["maxH"] = (float)m_maxTowerHeight.value();
  j["valToH"] = (float)m_fixedValToHeight.value();

  return ret;
}

void FWViewEnergyScale::ScaleChanged(const char *arg)
{
  using namespace nlohmann;
  std::string msg(TBase64::Decode(arg).Data());
  json j = json::parse(msg);
  try
  {
    m_plotEt.set(j["plotEt"]);
    std::string sm = j["mode"];
    m_scaleMode.set(FWViewEnergyScale::EScaleMode(atoi(sm.c_str())));
    m_maxTowerHeight.set(j["maxH"]);
    m_fixedValToHeight.set(j["valToH"]);
    parameterChanged_.emit();
  }
  catch (std::exception &e)
  {
    std::cout << "Exception in FWViewEnergyScale::ScaleChange " << e.what() << std::endl;
  }
}

void FWViewEnergyScale::addTo(FWConfiguration &oTo) const
{
  FWConfigurableParameterizable::addTo(oTo);
}

void FWViewEnergyScale::setFrom(const FWConfiguration& iFrom) {
  for (const_iterator it = begin(), itEnd = end(); it != itEnd; ++it) {
    (*it)->setFrom(iFrom);
    std::cout << "iterate \n";
  }
}
