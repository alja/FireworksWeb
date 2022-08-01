#ifndef FireworksWeb_Core_FWLongParameterSetter_h
// -*- C++ -*-
//
// Package:     Core
// Class  :     FWLongParameterSetter
//
/**\class FWLongParameterSetter FWLongParameterSetter.h Fireworks/Core/interface/FWLongParameterSetter.h

   Description: <one line class summary>

   Usage:
    <usage>

 */
//
// Original Author:  Chris Jones
//         Created:  Mon Mar 10 11:22:26 CDT 2008
//
class FWLongParamter;

// user include files
#include "FireworksWeb/Core/interface/FWParameterSetterBase.h"
#include "FireworksWeb/Core/interface/FWLongParameter.h"


class FWWebLongParameterSetter : public FWParameterSetterBase {
public:
  FWWebLongParameterSetter();
  ~FWWebLongParameterSetter() override;
  
  void writeJson(nlohmann::json&) override;

  FWWebLongParameterSetter(const FWWebLongParameterSetter&) = delete;  // stop default

  const FWWebLongParameterSetter& operator=(const FWWebLongParameterSetter&) = delete;  // stop default

private:
  // ---------- member data --------------------------------
  FWLongParameter* m_param{nullptr};
  void attach(FWParameterBase*) override;
  void setFromMIR(const char* val) override;
};

#endif
