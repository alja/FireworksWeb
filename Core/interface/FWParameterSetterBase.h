#ifndef FireworksWeb_Core_FWParameterSetterBase_h
#define FireworksWeb_Core_FWParameterSetterBase_h

#include <memory>

#include <nlohmann/json.hpp>

// -*- C++ -*-
//
// Package:     Core
// Class  :     FWParameterSetterBase
//
/**\class FWParameterSetterBase FWParameterSetterBase.h FireworksWeb/Core/interface/FWParameterSetterBase.h

   Description: <one line class summary>

   Usage:
    <usage>

 */
//
// Original Author:  Chris Jones
//         Created:  Fri Mar  7 14:16:14 EST 2008
//


// user include files

// forward declarations
class FWParameterBase;

class FWParameterSetterBase
{
public:
  FWParameterSetterBase();
  virtual ~FWParameterSetterBase();

  // ---------- const member functions ---------------------

  // ---------- static member functions --------------------

  static std::shared_ptr<FWParameterSetterBase> makeSetterFor(FWParameterBase *);

  // ---------- member functions ---------------------------

  virtual void writeJson(nlohmann::json &) = 0;
  virtual void attach(FWParameterBase *p) = 0;
  virtual void setFromMIR(const char* val) = 0;

private:
  FWParameterSetterBase(const FWParameterSetterBase &) = delete;                  // stop default
  const FWParameterSetterBase &operator=(const FWParameterSetterBase &) = delete; // stop default
};

#endif
