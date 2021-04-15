#ifndef FireworksWeb_Core_FWProxyBuilderConfiguration_h
#define FireworksWeb_Core_FWProxyBuilderConfiguration_h
// -*- C++ -*-
//
// Package:     Core
// Class  :     FWProxyBuilderConfiguration
//
/**\class FWProxyBuilderConfiguration FWProxyBuilderConfiguration.h FireworksWeb/Core/interface/FWProxyBuilderConfiguration.h

 Description: [one line class summary]

 Usage:
    <usage>

*/
//
// Original Author:
//         Created:  Wed Jul 27 00:58:35 CEST 2011
//

#include <string>
#include "FireworksWeb/Core/interface/FWConfigurableParameterizable.h"
// #include "FireworksWeb/Core/interface/FWViewType.h"
// #include "FireworksWeb/Core/interface/FWParameterSetterBase.h"
// #include "FireworksWeb/Core/interface/FWParameterSetterEditorBase.h"

#include "FireworksWeb/Core/interface/FWParameters.h"
#include "FireworksWeb/Core/interface/FWGenericParameterWithRange.h"

#ifndef __CINT__
#include <memory>
#include <sigc++/sigc++.h>
#endif


class FWParameterBase;
class FWConfiguration;
class FWEventItem;

//==============================================================================
class FWProxyBuilderConfiguration : public FWConfigurableParameterizable/*, public FWParameterSetterEditorBase*/ {
public:
  FWProxyBuilderConfiguration(const FWConfiguration* c, const FWEventItem* item);
  ~FWProxyBuilderConfiguration() override;

  template <class T>
  FWGenericParameter<T>* assertParam(const std::string& name, T def);
  template <class T>
  FWGenericParameterWithRange<T>* assertParam(const std::string& name, T def, T min, T max);
  template <class T>
  T value(const std::string& name);

  void setFrom(const FWConfiguration& iFrom) override;
  void addTo(FWConfiguration& iTo) const override;

   // void populateFrame(TGCompositeFrame* frame);

  void keepEntries(bool b);

private:
   // void makeSetter(TGCompositeFrame*, FWParameterBase*);

  const FWConfiguration* m_txtConfig;
  const FWEventItem* m_item;

  bool m_keepEntries;
#ifndef __CINT__
   //std::vector<std::shared_ptr<FWParameterSetterBase> > m_setters;
#endif
};
#endif
