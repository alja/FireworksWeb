#ifndef Fireworks2_Core_FWEventItem_h
#define Fireworks2_Core_FWEventItem_h
// -*- C++ -*-
//
// Package:     Core
// Class  :     FWEventItem
//
/**\class FWEventItem FWEventItem.h Fireworks2/Core/interface/FWEventItem.h

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
#include "Fireworks2/Core/interface/FWDisplayProperties.h"
#include "Fireworks2/Core/interface/FWPhysicsObjectDesc.h"
#include "Fireworks2/Core/interface/Context.h"

// forward declarations
class TClass;
class TVirtualCollectionProxy;
class FWItemAccessorBase;

namespace edm {
   class EventBase;
}
namespace fireworks {
   class Context;
}

namespace ROOT {
namespace Experimental {
   class REveDataCollection;
}
}

class FWEventItem
{
public:
   FWEventItem(std::shared_ptr<FWItemAccessorBase> iAccessor,
               const FWPhysicsObjectDesc& iDesc);
   virtual ~FWEventItem();

   ROOT::Experimental::REveDataCollection* getCollection() { return m_collection; }
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
   
private:
   std::shared_ptr<FWItemAccessorBase> m_accessor;
   ROOT::Experimental::REveDataCollection* m_collection;

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

   const void* data() const;
   void setData(const edm::ObjectWithDict& ) const;
   //void getPrimaryData() const;
};


#endif
