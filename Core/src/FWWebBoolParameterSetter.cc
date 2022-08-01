// -*- C++ -*-
//
// Package:     Core
// Class  :     FWWebBoolParameterSetter
//
// Implementation:
//     <Notes on implementation>
//
// Original Author:  Chris Jones
//         Created:  Mon Mar 10 11:22:32 CDT 2008
//
#include <iostream>

// user include files
#include "FireworksWeb/Core/src/FWWebBoolParameterSetter.h"
#include "nlohmann/json.hpp"

//
// constructors and destructor
//
FWWebBoolParameterSetter::FWWebBoolParameterSetter() {}

// FWWebBoolParameterSetter::FWWebBoolParameterSetter(const FWWebBoolParameterSetter& rhs)
// {
//    // do actual copying here;
// }

FWWebBoolParameterSetter::~FWWebBoolParameterSetter() {}

//
// member functions
//

void FWWebBoolParameterSetter::attach(FWParameterBase* iParam)
{
  m_param = dynamic_cast<FWBoolParameter*>(iParam);
}

void FWWebBoolParameterSetter::writeJson(nlohmann::json& j)
{
  std::cout << "FWWebBoolParameterSetter::writeJson " << (void*)m_param << "\n";
  if (m_param == nullptr)
  return;

  j["name"] = m_param->name();
  j["value"] = m_param->value();
  j["type"] = "Bool";
}


void FWWebBoolParameterSetter::setFromMIR(const char* mval)
{
  if (strcmp(mval, "true") == 0)
  m_param->set(true);
  else
  m_param->set(false);

}
