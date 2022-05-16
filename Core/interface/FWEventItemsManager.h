// -*- C++ -*-
#ifndef FireworksWeb_Core_FWEventItemsManager_h
#define FireworksWeb_Core_FWEventItemsManager_h
//
// Package:     Core
// Class  :     FWEventItemsManager
//
/**\class FWEventItemsManager FWEventItemsManager.h FireworksWeb/Core/interface/FWEventItemsManager.h

   Description: Manages multiple FWEventItems

   Usage:
    <usage>

 */
//
// Original Author:
//         Created:  Thu Jan  3 13:27:29 EST 2008
//

// system include files
#include <vector>
#include <memory>
#include "sigc++/signal.h"

// user include files
#include "FireworksWeb/Core/interface/FWConfigurable.h"

// forward declarations
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
class FWEventItem;
class FWPhysicsObjectDesc;
class FWItemAccessorFactory;

class FWEventItemsManager : public FWConfigurable {
public:
  //does not take ownership of the object to which it points but does keep reference
  FWEventItemsManager();
  ~FWEventItemsManager() override;

  typedef std::vector<FWEventItem*>::const_iterator const_iterator;

  //configuration management interface
  void addTo(FWConfiguration&) const override;
  void setFrom(const FWConfiguration&) override;

  // ---------- const member functions ---------------------
  ///NOTE: iterator is allowed to return a null object for items that have been removed
  const_iterator begin() const;
  const_iterator end() const;
  // const std::vector<FWEventItem*> &items () const { return m_items; }

  //const FWEventItem* find(const std::string& iName) const;
  const FWEventItem* find(const ROOT::Experimental::REveDataCollection*) const;
  // ---------- static member functions --------------------

  // ---------- member functions ---------------------------
  FWEventItem* add(const FWPhysicsObjectDesc& iItem, const FWConfiguration* pbConf = nullptr, bool doSetEvent = true);
  void clearItems();

   size_t getNumItems() {return m_items.size();}

  void newEvent(const edm::EventBase* iEvent);
   
   sigc::signal<void(FWEventItem*)> newItem_;

   //const FWEventIntem* getItemForEveCollection(ROOT::Experimetal::REveDataCollection*) const;

private:
  void removeItem(const FWEventItem*);
  FWEventItemsManager(const FWEventItemsManager&) = delete;  // stop default

  const FWEventItemsManager& operator=(const FWEventItemsManager&) = delete;  // stop default

  // ---------- member data --------------------------------
  std::vector<FWEventItem*> m_items;

  const edm::EventBase* m_event;
  std::shared_ptr<FWItemAccessorFactory> m_accessorFactory;
};

#endif
