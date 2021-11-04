
#include <iostream>

#include "Rtypes.h"
#include "TMath.h"
#include "FireworksWeb/Core/interface/FWEveView.h"
#include "FireworksWeb/Core/interface/FWViewEnergyScale.h"
#include "FireworksWeb/Core/interface/Context.h"
 
#include "TBase64.h"

FWViewEnergyScale::FWViewEnergyScale(std::string name):
      m_scaleMode(kAutoScale),
      m_fixedValToHeight(50.0),
      m_maxTowerHeight(3.0),
      m_plotEt(true),
      m_name(name),
      m_scaleFactor3D(1.f),
      m_scaleFactorLego(0.05f) {

      SetName("EnergyScale");
/*
  m_scaleMode.changed_.connect(std::bind(&FWViewEnergyScale::scaleParameterChanged, this));
  m_fixedValToHeight.changed_.connect(std::bind(&FWViewEnergyScale::scaleParameterChanged, this));
  m_maxTowerHeight.changed_.connect(std::bind(&FWViewEnergyScale::scaleParameterChanged, this));
  m_plotEt.changed_.connect(std::bind(&FWViewEnergyScale::scaleParameterChanged, this));
  */
}

FWViewEnergyScale::~FWViewEnergyScale() {}

//________________________________________________________

void FWViewEnergyScale::scaleParameterChanged() const { parameterChanged_.emit(); }

float FWViewEnergyScale::calculateScaleFactor(float iMaxVal, bool isLego) const {
  // check if in combined mode
  int mode = m_scaleMode;
  if (mode == kCombinedScale) {
    mode = (m_maxTowerHeight > 100 * iMaxVal / m_fixedValToHeight) ? kFixedScale : kAutoScale;
    // printf("COMBINED  \n");
  }
  // get converison

  if (mode == kFixedScale) {
    //  printf("fixed mode %f \n",m_fixedValToHeight.);
    // apply default constructor height
    float length = isLego ? TMath::Pi() : 100;
    return length / m_fixedValToHeight;
  } else {
    float length = isLego ? TMath::Pi() : (100 * m_maxTowerHeight);
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

  j["plotEt"] = m_plotEt;
  j["mode"] = std::to_string(m_scaleMode);
  j["maxH"] = m_maxTowerHeight;
  j["valToH"] = m_fixedValToHeight;

  return ret;
}

void FWViewEnergyScale::ScaleChanged(const char *arg)
{
  using namespace nlohmann;
  std::string msg(TBase64::Decode(arg).Data());
  json j = json::parse(msg);
  try
  {
    m_plotEt = j["plotEt"];
    std::string sm = j["mode"];
    m_scaleMode = FWViewEnergyScale::EScaleMode(atoi(sm.c_str()));
    m_maxTowerHeight = j["maxH"];
    m_fixedValToHeight = j["valToH"];
    parameterChanged_.emit();
  }
  catch (std::exception &e)
  {
    std::cout << "Exception in FWViewEnergyScale::ScaleChange " << e.what() << std::endl;
  }
}

/*
void FWViewEnergyScale::setFrom(const FWConfiguration& iFrom) {
  for (const_iterator it = begin(), itEnd = end(); it != itEnd; ++it) {
    (*it)->setFrom(iFrom);
  }
}
void FWViewEnergyScale::SetFromCmsShowCommonConfig(long mode, float convert, float maxH, bool et) {
  m_scaleMode.set(mode);
  m_fixedValToHeight.set(convert);
  m_maxTowerHeight.set(maxH);
  m_plotEt.set(et > 0);
}*/
