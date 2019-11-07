// -*- C++ -*-
#ifndef Fireworks2_Core_FWEventItemsManager_h
#define Fireworks2_Core_FWEventItemsManager_h
//
// Package:     Core
// Class  :     FWEventItemsManager
//
/**\class FWEventItemsManager FWEventItemsManager.h Fireworks/Core/interface/FWEventItemsManager.h

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

// user include files
#include "Fireworks2/Core/interface/FWConfigurable.h"

// forward declarations
namespace edm {
  class EventBase;
}
namespace fireworks {
  class Context;
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

  const FWEventItem* find(const std::string& iName) const;
  // ---------- static member functions --------------------

  // ---------- member functions ---------------------------
  FWEventItem* add(const FWPhysicsObjectDesc& iItem, const FWConfiguration* pbConf = nullptr, bool doSetEvent = true);
  void clearItems();

  void newEvent(const edm::EventBase* iEvent);
   
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
