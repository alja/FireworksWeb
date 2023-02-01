// -*- C++ -*-
//
// Package:     Core
// Class  :     FWWebLongParameterSetter
//
// Implementation:
//     <Notes on implementation>
//
// Original Author:  Chris Jones
//         Created:  Mon Mar 10 11:22:32 CDT 2008
//
#include <iostream>

// user include files
#include "FireworksWeb/Core/src/FWWebLongParameterSetter.h"
#include "nlohmann/json.hpp"

//
// constructors and destructor
//
FWWebLongParameterSetter::FWWebLongParameterSetter() {}

// FWWebLongParameterSetter::FWWebLongParameterSetter(const FWWebLongParameterSetter& rhs)
// {
//    // do actual copying here;
// }

FWWebLongParameterSetter::~FWWebLongParameterSetter() {}

//
// member functions
//

void FWWebLongParameterSetter::attach(FWParameterBase* iParam)
{
  m_param = dynamic_cast<FWLongParameter*>(iParam);
}

void FWWebLongParameterSetter::writeJson(nlohmann::json& j)
{
  // std::cout << "FWWebLongParameterSetter::writeJson " << (void*)m_param << "\n";
  if (m_param == nullptr)
  return;

  j["name"] = m_param->name();
  j["val"] = m_param->value();
  j["type"] = "Long";
  j["min"] =  m_param->min();
  j["max"] =  m_param->max();
}


void FWWebLongParameterSetter::setFromMIR(const char* mval)
{
  char *eptr;
  long val = strtol(mval, &eptr, 10);
  if (errno == EINVAL)
  {
      printf("Conversion error occurred: %d\n", errno);
      return;
  }

printf("FWWebLongParameterSetter::setFromMIR Mir value in decimal format %ld \n", val);
  m_param->set(val);
}
