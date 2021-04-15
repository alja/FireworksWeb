#ifndef FireworksWeb_Core_FWParameters_h
#define FireworksWeb_Core_FWParameters_h
// -*- C++ -*-
//
// Package:     Core
// Class  :     FWGenericParameterWithRange
//
/**\class FWGenericParameterWithRange FWGenericParameter.h FireworksWeb/Core/interface/FWLongParameter.h

   Description: Provides access to a simple double parameter

   Usage:
    If min and max values are both identical than no restriction is placed on the allowed value

 */
//
// Original Author:  Chris Jones
//         Created:  Fri Mar  7 14:36:34 EST 2008
//

// user include files
#include "FireworksWeb/Core/interface/FWGenericParameter.h"
#include "FireworksWeb/Core/interface/FWGenericParameterWithRange.h"

// forward declarations

struct FWParameters {
  typedef FWGenericParameterWithRange<long> Long;
  typedef FWGenericParameterWithRange<double> Double;
  typedef FWGenericParameter<std::string> String;
  typedef FWGenericParameter<bool> Bool;
};

typedef FWParameters::Long FWLongParameter;
typedef FWParameters::Double FWDoubleParameter;
typedef FWParameters::String FWStringParameter;
typedef FWParameters::Bool FWBoolParameter;

#endif
