// -*- C++ -*-
//
// Package:     Core
// Class  :     FWProxyBuilderConfiguration
//
// Implementation:
//     [Notes on implementation]
//
// Original Author:
//         Created:  Wed Jul 27 00:58:43 CEST 2011
//

// system include files

// user include files
#include <iostream>
#include <stdexcept>
#include <functional>
#include "FireworksWeb/Core/interface/FWParameterSetterBase.h"

#include "FireworksWeb/Core/interface/FWProxyBuilderConfiguration.h"
#include "FireworksWeb/Core/interface/FWWebEventItem.h"
#include "FireworksWeb/Core/interface/fwLog.h"
#include "FireworksWeb/Core/interface/FWConfiguration.h"
#include "FireworksWeb/Core/interface/FW2EveManager.h"
#include "FWCore/Utilities/interface/TypeID.h"

#include "FWCore/Reflection/interface/TypeWithDict.h"

FWProxyBuilderConfiguration::FWProxyBuilderConfiguration(const FWConfiguration* c, const FWWebEventItem* item, FW2EveManager* em)
    : m_txtConfig(c), m_item(item), m_keepEntries(false), m_eveMng(em) {}

FWProxyBuilderConfiguration::~FWProxyBuilderConfiguration() { delete m_txtConfig; }

//______________________________________________________________________________

void FWProxyBuilderConfiguration::addTo(FWConfiguration& iTo) const {
  if (begin() != end()) {
    FWConfiguration vTmp;
    FWConfigurableParameterizable::addTo(vTmp);
    iTo.addKeyValue("Var", vTmp, true);
  }
}

void FWProxyBuilderConfiguration::setFrom(const FWConfiguration& iFrom) {
  /*
     for(FWConfiguration::KeyValuesIt it = keyVals->begin(); it!= keyVals->end(); ++it)
     std::cout << it->first << "FWProxyBuilderConfiguration::setFrom  " << std::endl;
     }*/
}

//______________________________________________________________________________

template <class T>
FWGenericParameter<T> *FWProxyBuilderConfiguration::assertParam(const std::string &name, T def)
{
  printf("assert parameter %s \n", name.c_str());
  for (const_iterator i = begin(); i != end(); ++i)
  {
    printf("Assert paramter comapre exisitnf %s \n", (*i)->name().c_str());
    if ((*i)->name() == name)
    {
      return nullptr;
    }
  }

  FWGenericParameter<T> *mode = new FWGenericParameter<T>(this, name, def);

  std::cout << "FWProxyBuilderConfiguration::getVarParameter(). No parameter with name " << name << std::endl;
  if (m_txtConfig)
  {
    const FWConfiguration *varConfig = m_txtConfig->keyValues() ? m_txtConfig->valueForKey("Var") : nullptr;
    if (varConfig)
      mode->setFrom(*varConfig);
  }
  mode->changed_.connect(std::bind(&FW2EveManager::itemConfigChanged, m_eveMng, (FWWebEventItem *)m_item));
  std::shared_ptr<FWParameterSetterBase> ptr(FWParameterSetterBase::makeSetterFor(mode));

  ptr->attach(mode);
  m_setters.push_back(ptr);

  return mode;
}

//------------------------------------------------------------------------------

template <class T>
FWGenericParameterWithRange<T> *FWProxyBuilderConfiguration::assertParam(const std::string &name, T def, T min, T max)
{
  for (const_iterator i = begin(); i != end(); ++i)
  {
    if ((*i)->name() == name)
    {
      return nullptr;
    }
  }

  FWGenericParameterWithRange<T> *mode = new FWGenericParameterWithRange<T>(this, name, def, min, max);

  //   std::cout << "FWProxyBuilderConfiguration::getVarParameter(). No parameter with name " << name << std::endl;
  const FWConfiguration *varConfig =
      m_txtConfig && m_txtConfig->keyValues() ? m_txtConfig->valueForKey("Var") : nullptr;
  if (varConfig)
    mode->setFrom(*varConfig);
  
  mode->changed_.connect(std::bind(&FW2EveManager::itemConfigChanged, m_eveMng, (FWWebEventItem *)m_item));
  //mode->changed_.connect(std::bind(&FWWebEventItem::proxyConfigChanged, (FWWebEventItem *)m_item, m_keepEntries));

  // amt (1)
  std::shared_ptr<FWParameterSetterBase> ptr(FWParameterSetterBase::makeSetterFor(mode));
  ptr->attach(mode);
  m_setters.push_back(ptr);


  return mode;
}

//------------------------------------------------------------------------------
template <class T>
T FWProxyBuilderConfiguration::value(const std::string& pname) {
  FWGenericParameter<T>* param = nullptr;

  for (FWConfigurableParameterizable::const_iterator i = begin(); i != end(); ++i) {
    if ((*i)->name() == pname) {
      param = (FWGenericParameter<T>*)(*i);
      break;
    }
  }

  if (param)
    return param->value();
  else
    throw std::runtime_error("Invalid parameter request.");
}

//------------------------------------------------------------------------------
void FWProxyBuilderConfiguration::writeJson(nlohmann::json &j) const
{
  if (m_setters.empty())
    return;

  nlohmann::json jarr = nlohmann::json::array();
  for (auto &i : m_setters)
  {
    nlohmann::json so = {};
    i->writeJson(so);
    jarr.push_back(so);
  }
  j["var"] = jarr;

  std::cout << "dddddd FWProxyBuilderConfiguration::writeJson" << j.dump(3);
}


//------------------------------------------------------------------------------
void FWProxyBuilderConfiguration::setFromMIR(const char *name, const char* value)
{
  std::string pname = name;

  auto sit = m_setters.begin();
  for (FWConfigurableParameterizable::const_iterator i = begin(); i != end(); ++i, ++sit)
  {
    if ((*i)->name() == pname)
       (*sit)->setFromMIR(value);
  }
}

// explicit template instantiation

template bool FWProxyBuilderConfiguration::value<bool>(const std::string& name);
template long FWProxyBuilderConfiguration::value<long>(const std::string& name);
template double FWProxyBuilderConfiguration::value<double>(const std::string& name);

template FWGenericParameter<bool>* FWProxyBuilderConfiguration::assertParam(const std::string& name, bool def);
template FWGenericParameterWithRange<long>* FWProxyBuilderConfiguration::assertParam(const std::string& name,
                                                                                     long def,
                                                                                     long min,
                                                                                     long max);
template FWGenericParameterWithRange<double>* FWProxyBuilderConfiguration::assertParam(const std::string& name,
                                                                                       double def,
                                                                                       double min,
                                                                                       double max);
