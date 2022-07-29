#ifndef FireworksWeb_Core_FWBoolParameterSetter_h
// -*- C++ -*-
//
// Package:     Core
// Class  :     FWBoolParameterSetter
//
/**\class FWBoolParameterSetter FWBoolParameterSetter.h Fireworks/Core/interface/FWBoolParameterSetter.h

   Description: <one line class summary>

   Usage:
    <usage>

 */
//
// Original Author:  Chris Jones
//         Created:  Mon Mar 10 11:22:26 CDT 2008
//
class FWBoolParamter;

// user include files
#include "FireworksWeb/Core/interface/FWParameterSetterBase.h"
#include "FireworksWeb/Core/interface/FWBoolParameter.h"


class FWWebBoolParameterSetter : public FWParameterSetterBase {
public:
  FWWebBoolParameterSetter();
  ~FWWebBoolParameterSetter() override;
  
  void writeJson(nlohmann::json&) override;

  FWWebBoolParameterSetter(const FWWebBoolParameterSetter&) = delete;  // stop default

  const FWWebBoolParameterSetter& operator=(const FWWebBoolParameterSetter&) = delete;  // stop default

private:
  // ---------- member data --------------------------------
  FWBoolParameter* m_param{nullptr};
  void attach(FWParameterBase*) override;
  void setFromMIR(const char* val) override;
};

#endif
