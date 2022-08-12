// -*- C++ -*-
//
// Package:     Core
// Class  :     FWWebDoubleParameterSetter
//
// Implementation:
//     <Notes on implementation>
//
// Original Author:  Chris Jones
//         Created:  Mon Mar 10 11:22:32 CDT 2008
//
#include <iostream>

// user include files
#include "FireworksWeb/Core/src/FWWebDoubleParameterSetter.h"
#include "nlohmann/json.hpp"

//
// constructors and destructor
//
FWWebDoubleParameterSetter::FWWebDoubleParameterSetter() {}

// FWWebDoubleParameterSetter::FWWebDoubleParameterSetter(const FWWebDoubleParameterSetter& rhs)
// {
//    // do actual copying here;
// }

FWWebDoubleParameterSetter::~FWWebDoubleParameterSetter() {}

//
// member functions
//

void FWWebDoubleParameterSetter::attach(FWParameterBase* iParam)
{
  m_param = dynamic_cast<FWDoubleParameter*>(iParam);
}

void FWWebDoubleParameterSetter::writeJson(nlohmann::json& j)
{
  std::cout << "FWWebDoubleParameterSetter::writeJson " << (void*)m_param << "\n";
  if (m_param == nullptr)
  return;

  j["name"] = m_param->name();
  j["val"] = m_param->value();
  j["type"] = "Double";
  j["min"] =  m_param->min();
  j["max"] =  m_param->max();
}


void FWWebDoubleParameterSetter::setFromMIR(const char* mval)
{
  std::string s = mval;
  try {
    double val = std::stod(s);
    printf("FWWebDoubleParameterSetter::setFromMIR Mir value in decimal format %f \n", val);
    m_param->set(val);
  }
  catch (std::exception &e) {
    std::cerr << "FWWebDoubleParameterSetter::setFromMIR " << e.what() << "\n";
  }
}
