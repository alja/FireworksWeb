// -*- C++ -*-
#ifndef FireworksWeb_Core_FWWebEventItemsManager_h
#define FireworksWeb_Core_FWWebEventItemsManager_h
//
// Package:     Core
// Class  :     FWWebEventItemsManager
//
/**\class FWWebEventItemsManager FWWebEventItemsManager.h FireworksWeb/Core/interface/FWWebEventItemsManager.h

   Description: Manages multiple FWWebEventItems

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
class FWWebEventItem;
class FWPhysicsObjectDesc;
class FWItemAccessorFactory;
class FW2EveManager;

class FWWebEventItemsManager : public FWConfigurable {
public:
  //does not take ownership of the object to which it points but does keep reference
  FWWebEventItemsManager(FW2EveManager*);
  ~FWWebEventItemsManager() override;

  typedef std::vector<FWWebEventItem*>::const_iterator const_iterator;

  //configuration management interface
  void addTo(FWConfiguration&) const override;
  void setFrom(const FWConfiguration&) override;

  // ---------- const member functions ---------------------
  ///NOTE: iterator is allowed to return a null object for items that have been removed
  const_iterator begin() const;
  const_iterator end() const;
  // const std::vector<FWWebEventItem*> &items () const { return m_items; }

  //const FWWebEventItem* find(const std::string& iName) const;
  const FWWebEventItem* find(const ROOT::Experimental::REveDataCollection*) const;
  // ---------- static member functions --------------------

  // ---------- member functions ---------------------------
  FWWebEventItem* add(const FWPhysicsObjectDesc& iItem, const FWConfiguration* pbConf = nullptr, bool doSetEvent = true);
  void clearItems();

   size_t getNumItems() {return m_items.size();}

  void newEvent(const edm::EventBase* iEvent);
   
   sigc::signal<void(FWWebEventItem*)> newItem_;

   //const FWEventIntem* getItemForEveCollection(ROOT::Experimetal::REveDataCollection*) const;

private:
  void removeItem(const FWWebEventItem*);
  FWWebEventItemsManager(const FWWebEventItemsManager&) = delete;  // stop default

  const FWWebEventItemsManager& operator=(const FWWebEventItemsManager&) = delete;  // stop default

  // ---------- member data --------------------------------
  std::vector<FWWebEventItem*> m_items;

  const edm::EventBase* m_event;
  std::shared_ptr<FWItemAccessorFactory> m_accessorFactory;

  FW2EveManager* m_fwEveMng{nullptr};
};

#endif
