#ifndef FireworksWeb_Core_FWWebEventItem_h
#define FireworksWeb_Core_FWWebEventItem_h
// -*- C++ -*-
//
// Package:     Core
// Class  :     FWWebEventItem
//
/**\class FWWebEventItem FWWebEventItem.h FireworksWeb/Core/interface/FWWebEventItem.h

   Description: Stand in for a top level item in an Event

   Usage:
    <usage>

 */
//
// Original Author:  Chris Jones
//         Created:  Thu Jan  3 14:02:21 EST 2008
//

// system include files
#include <string>
#include <vector>
#include <memory>
#include "FWCore/Reflection/interface/TypeWithDict.h"

// user include files
#include "FireworksWeb/Core/interface/FWDisplayProperties.h"
#include "FireworksWeb/Core/interface/FWPhysicsObjectDesc.h"
#include "FireworksWeb/Core/interface/Context.h"
#include "ROOT/REveDataCollection.hxx"

// forward declarations
class TClass;
class TVirtualCollectionProxy;
class FWItemAccessorBase;
class FWProxyBuilderConfiguration;
class FW2EveManager;
class FWConfiguration;

namespace edm {
   class EventBase;
}
namespace fireworks {
   class Context;
}



class FWWebEventItem : public ROOT::Experimental::REveDataCollection
{
public:
   FWWebEventItem(std::shared_ptr<FWItemAccessorBase> iAccessor,
               const FWPhysicsObjectDesc& iDesc, const FWConfiguration* pbc, FW2EveManager* iEM);
   virtual ~FWWebEventItem();

   ROOT::Experimental::REveDataCollection* getCollection() { return this; }
   // const void* modelData(int iIndex) const;

   // bool isCollection() const;
   void setEvent(const edm::EventBase* iEvent);
   const TClass* modelType() const;

   const TClass* type() const;
   const std::string& purpose() const;

   const std::string& moduleLabel() const;
   const std::string& productInstanceLabel() const;
   const std::string& processName() const;

   const char* name() const;
   const char* filterExpression();
   
   const FWDisplayProperties& defaultDisplayProperties() const;
   
   FWProxyBuilderConfiguration* getConfig() const { return m_proxyBuilderConfig; }
   
   // void proxyConfigChanged(bool k = false);

   const void* data();

   int WriteCoreJson(nlohmann::json &j, int rnr_offset) override;

   void UpdatePBParameter(char *name, char *val);
#if !defined(__CINT__) && !defined(__MAKECINT__)
  template <class T>
  void get(const T*& oData) {
    oData = reinterpret_cast<const T*>(data());
  }
#endif
private:
   std::shared_ptr<FWItemAccessorBase> m_accessor;
   std::string m_name; // AMT is this block of memebers necessary
   const TClass* m_type;
   std::string m_purpose;
   FWDisplayProperties m_displayProperties;
   std::string m_moduleLabel;
   std::string m_productInstanceLabel;
   std::string m_processName;

   const edm::EventBase* m_event;

   mutable bool m_printedErrorThisEvent;
   mutable std::string m_errorMessage;

   void setData(const edm::ObjectWithDict& );
   //void getPrimaryData() const;
   FWProxyBuilderConfiguration*  m_proxyBuilderConfig;
};


#endif
