#ifndef FireworksWeb_Core_register_associaton_macro_h
#define FireworksWeb_Core_register_association_macro_h

// system include files
#include <cstdlib>
#include "boost/lexical_cast.hpp"
#include "FWCore/Reflection/interface/TypeWithDict.h"

// forward declarations

#define REGISTER_FWASSOCIATION_METHODS() \
   using FWAssociationBase::associatable; \
   virtual const std::string& associatable() const { return classAssociationA(); } \
   using FWAssociationBase::associated; \
   virtual const std::string& associated() const { return classAssociationB(); } \
   const std::string& typeName() const { return classTypeName(); } \
   static const std::string& classRegisterTypeName(); \
   static const std::string& classTypeName(); \
   static const std::string& classAssociationA(); \
   static const std::string& classAssociationB()

#define CONCATENATE_HIDDEN(a,b) a ## b
#define CONCATENATE(a,b) CONCATENATE_HIDDEN(a,b)

#define DEFINE_FWASSOCIATION_METHODS(_builder_,_type_,_associationA_, _associationB_)	\
   const std::string& _builder_::classTypeName() { \
      static std::string s_type = edm::TypeWithDict(typeid(_type_)).name(); \
      return s_type;} \
   const std::string& _builder_::classRegisterTypeName() { \
      static std::string s_type( typeid(_type_).name() ); \
      return s_type;} \
   const std::string& _builder_::classAssociationA(){ \
      static std::string s_associationA(_associationA_); return s_associationA;} \
   const std::string& _builder_::classAssociationB(){ \
      static std::string s_associationB(_associationB_); return s_associationB;} \
   enum {CONCATENATE(dummy_association_methods_, __LINE__)}

#endif

