#ifndef FireworksWeb_Core_FWDoublrParameterSetter_h
// -*- C++ -*-
//
// Package:     Core
// Class  :     FWDoubleParameterSetter
//
/**\class FWDoubleParameterSetter FWDoubleParameterSetter.h Fireworks/Core/interface/FWDoubleParameterSetter.h

   Description: <one line class summary>

   Usage:
    <usage>

 */
//
// Original Author:  Chris Jones
//         Created:  Mon Mar 10 11:22:26 CDT 2008
//
class FWDoubleParamter;

// user include files
#include "FireworksWeb/Core/interface/FWParameterSetterBase.h"
#include "FireworksWeb/Core/interface/FWDoubleParameter.h"


class FWWebDoubleParameterSetter : public FWParameterSetterBase {
public:
  FWWebDoubleParameterSetter();
  ~FWWebDoubleParameterSetter() override;
  
  void writeJson(nlohmann::json&) override;

  FWWebDoubleParameterSetter(const FWWebDoubleParameterSetter&) = delete;  // stop default

  const FWWebDoubleParameterSetter& operator=(const FWWebDoubleParameterSetter&) = delete;  // stop default

private:
  // ---------- member data --------------------------------
  FWDoubleParameter* m_param{nullptr};
  void attach(FWParameterBase*) override;
  void setFromMIR(const char* val) override;
};

#endif
