#ifndef FireworksWeb_Core_FWProxyBuilderFactory_h
#define FireworksWeb_Core_FWProxyBuilderFactory_h

// user include files
#include "ROOT/REveDataProxyBuilderBase.hxx"
#include "FireworksWeb/Core/interface/register_dataproxybuilder_macro.h"
#include "FWCore/PluginManager/interface/PluginFactory.h"

// forward declarations

class FWProxyBuilderBase;

typedef edmplugin::PluginFactory<ROOT::Experimental::REveDataProxyBuilderBase*()> FWProxyBuilderFactory;


#define REGISTER_FW2PROXYBUILDER(_name_,_type_,_purpose_) \
   DEFINE_FWPB_METHODS(_name_,_type_,_purpose_); \
   DEFINE_EDM_PLUGIN(FWProxyBuilderFactory, _name_, "simple#"+_name_::classRegisterTypeName() + "@" + _name_::classPurpose() + "@"+_name_::classView()+"#" # _name_)

#define REGISTER_FW2PROXYBUILDER_BASE(_name_,_type_,_purpose_) \
   DEFINE_FWPB_METHODS(_name_,_type_,_purpose_); \
   DEFINE_EDM_PLUGIN(FWProxyBuilderFactory, _name_, _name_::classRegisterTypeName() + "@" + _name_::classPurpose() + "@"+_name_::classView()+"#" # _name_)

#endif
