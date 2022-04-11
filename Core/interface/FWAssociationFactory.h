#ifndef FireworksWeb_Core_FWAssociationFactory_h
#define FireworksWeb_Core_FWAssociationFactory_h

// user include files
#include "FireworksWeb/Core/interface/FWAssociationProxyBase.h"
#include "FireworksWeb/Core/interface/register_association_macro.h"
#include "FWCore/PluginManager/interface/PluginFactory.h"

// forward declarations

typedef edmplugin::PluginFactory<FWAssociationProxyBase*()> FWAssociationFactory;


#define REGISTER_FWASSOCIATION(_builder_,_type_,_purpose_,_associationA_,_associationB_) \
   DEFINE_FWASSOCIATION_METHODS(_builder_,_type_,_purpose_,_associationA_,_associationB_); \
   DEFINE_EDM_PLUGIN(FWAssociationFactory, _builder_, _builder_::classRegisterTypeName() + "@" + _builder_::classPurpose()+ "#" # _builder_)

#endif
