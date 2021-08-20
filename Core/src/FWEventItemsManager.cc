// -*- C++ -*-
//
// Package:     Core
// Class  :     FWEventItemsManager
//
// Implementation:
//     <Notes on implementation>
//
// Original Author:
//         Created:  Fri Jan  4 10:38:18 EST 2008
//

// system include files
#include <sstream>
#include "TClass.h"
#include "TROOT.h"
#include <ROOT/REveDataCollection.hxx>

// user include files
#include "FireworksWeb/Core/interface/FWEventItemsManager.h"
#include "FireworksWeb/Core/interface/FWEventItem.h"
#include "FireworksWeb/Core/interface/FWConfiguration.h"
#include "FireworksWeb/Core/interface/FWDisplayProperties.h"
#include "FireworksWeb/Core/interface/FWItemAccessorFactory.h"
#include "FireworksWeb/Core/interface/fwLog.h"
#include <cassert>

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
FWEventItemsManager::FWEventItemsManager()
    :   m_event(nullptr), m_accessorFactory(new FWItemAccessorFactory()) {}

// FWEventItemsManager::FWEventItemsManager(const FWEventItemsManager& rhs)
// {
//    // do actual copying here;
// }

/** FWEventItemsManager has ownership of the items it contains.

    Note that because of the way we keep track of removed items,
    m_items[i] could actually be 0 for indices corresponding
    to removed items.
 */
FWEventItemsManager::~FWEventItemsManager() {
  for (size_t i = 0, e = m_items.size(); i != e; ++i)
    delete m_items[i];

  m_items.clear();
}

//
// assignment operators
//
// const FWEventItemsManager& FWEventItemsManager::operator=(const FWEventItemsManager& rhs)
// {
//   //An exception safe implementation is
//   FWEventItemsManager temp(rhs);
//   swap(rhs);
//
//   return *this;
// }

//
// member functions
//
FWEventItem* FWEventItemsManager::add(const FWPhysicsObjectDesc& iItem, const FWConfiguration* pbc, bool /*doSetEvent*/) {
  FWPhysicsObjectDesc temp(iItem);

  FWDisplayProperties prop(temp.displayProperties());
  temp.setDisplayProperties(prop);
  m_items.push_back(new FWEventItem(m_accessorFactory->accessorFor(temp.type()), temp));
  newItem_(m_items.back());
  return m_items.back();
}

/** Prepare to handle a new event by associating
    all the items to watch it.
  */
void FWEventItemsManager::newEvent(const edm::EventBase* iEvent) {
  m_event = iEvent;
  for (size_t i = 0, e = m_items.size(); i != e; ++i) {
    FWEventItem* item = m_items[i];
    if (item)
      item->setEvent(iEvent);
   }
}

/** Clear all the items in the model. 
    
    Notice that a previous implementation was setting all the items to 0, I
    guess to track accessing delete items.
  */
void FWEventItemsManager::clearItems(void) {
   /* AMT
  for (size_t i = 0, e = m_items.size(); i != e; ++i) {
    FWEventItem* item = m_items[i];
    if (item) {
      item->destroy();
    }
    m_items[i] = nullptr;
  }
  m_items.clear();
   */
}

static const std::string kType("type");
static const std::string kModuleLabel("moduleLabel");
static const std::string kProductInstanceLabel("productInstanceLabel");
static const std::string kProcessName("processName");
static const std::string kFilterExpression("filterExpression");
static const std::string kColor("color");
static const std::string kIsVisible("isVisible");
static const std::string kTrue("t");
static const std::string kFalse("f");
static const std::string kLayer("layer");
static const std::string kPurpose("purpose");
static const std::string kTransparency("transparency");

void FWEventItemsManager::addTo(FWConfiguration& iTo) const {
  for (std::vector<FWEventItem*>::const_iterator it = m_items.begin(); it != m_items.end(); ++it) {
    if (!*it)
      continue;
    FWConfiguration conf(6);
    edm::TypeWithDict dataType((*((*it)->type()->GetTypeInfo())));
    assert(dataType != edm::TypeWithDict());

    conf.addKeyValue(kType, FWConfiguration(dataType.name()));
    conf.addKeyValue(kModuleLabel, FWConfiguration((*it)->moduleLabel()));
    conf.addKeyValue(kProductInstanceLabel, FWConfiguration((*it)->productInstanceLabel()));
    conf.addKeyValue(kProcessName, FWConfiguration((*it)->processName()));
    conf.addKeyValue(kFilterExpression, FWConfiguration((*it)->filterExpression()));
    {
      std::ostringstream os;
      os << (*it)->getCollection()->GetMainColor();
      conf.addKeyValue(kColor, FWConfiguration(os.str()));
    }
    conf.addKeyValue(kIsVisible, FWConfiguration((*it)->getCollection()->GetRnrSelf() ? kTrue : kFalse));
    {
      std::ostringstream os;
      // os << (*it)->layer();
      os << 1;
      conf.addKeyValue(kLayer, FWConfiguration(os.str()));
    }
    conf.addKeyValue(kPurpose, (*it)->purpose());
    {
      std::ostringstream os;
      os << static_cast<int>((*it)->defaultDisplayProperties().transparency());
      conf.addKeyValue(kTransparency, FWConfiguration(os.str()));
    }
    /* AMT
    FWConfiguration pbTmp;
    (*it)->getConfig()->addTo(pbTmp);
    conf.addKeyValue("PBConfig", pbTmp, true);
    */
    iTo.addKeyValue((*it)->name(), conf, true);
  }
}

/** This is responsible for resetting the status of items from configuration  
  */
void FWEventItemsManager::setFrom(const FWConfiguration& iFrom) {

  clearItems();
  const FWConfiguration::KeyValues* keyValues = iFrom.keyValues();

  if (keyValues == nullptr)
    return;

  std::vector<FWEventItem*> newItems;
  newItems.reserve(keyValues->size());

  for (FWConfiguration::KeyValues::const_iterator it = keyValues->begin(); it != keyValues->end(); ++it) {
    const std::string& name = it->first;
    const FWConfiguration& conf = it->second;
    const FWConfiguration::KeyValues* keyValues = conf.keyValues();
    assert(nullptr != keyValues);
    const std::string& type = (*keyValues)[0].second.value();
    const std::string& moduleLabel = (*keyValues)[1].second.value();
    const std::string& productInstanceLabel = (*keyValues)[2].second.value();
    const std::string& processName = (*keyValues)[3].second.value();
    const std::string& filterExpression = (*keyValues)[4].second.value();
    const std::string& sColor = (*keyValues)[5].second.value();
    const bool isVisible = (*keyValues)[6].second.value() == kTrue;

    unsigned int colorIndex;
    if (conf.version() < 5) {
      std::istringstream is(sColor);
      Color_t color;
      is >> color;
      colorIndex = color;
    } else {
      // In version 4 we assume:
      //   fireworks colors start at ROOT index 1000
      //   geometry  colors start at ROOT index 1100
      // We save them as such -- no conversions needed.
      std::istringstream is(sColor);
      is >> colorIndex;
    }

    int transparency = 0;

    // Read transparency from file. We don't care about checking errors
    // because strtol returns 0 in that case.
    if (conf.version() > 3)
      transparency = strtol((*keyValues)[9].second.value().c_str(), nullptr, 10);

    FWDisplayProperties dp(colorIndex, isVisible, transparency);

    unsigned int layer = 1;//strtol((*keyValues)[7].second.value().c_str(), nullptr, 10);

    //For older configs assume name is the same as purpose
    std::string purpose(name);
    if (conf.version() > 1)
      purpose = (*keyValues)[8].second.value();

    FWConfiguration* proxyConfig =
        (FWConfiguration*)conf.valueForKey("PBConfig") ? new FWConfiguration(*conf.valueForKey("PBConfig")) : nullptr;

    FWPhysicsObjectDesc desc(name,
                             TClass::GetClass(type.c_str()),
                             purpose,
                             dp,
                             moduleLabel,
                             productInstanceLabel,
                             processName,
                             filterExpression,
                             layer);

    newItems.push_back(add(desc, proxyConfig, false));
  }

  std::stringstream ss;
  for (auto ip : newItems)
  {
    for (auto &t : ip->getCollection()->GetItemList()->RefToolTipExpressions())
      ss << t->fTooltipFunction.GetFunctionExpressionString();
  }
  gROOT->ProcessLine(ss.str().c_str());

  if (m_event)
  {
    for (auto ip : newItems)
      ip->setEvent(m_event);
  }
}

/** Remove one item. 
  */
void FWEventItemsManager::removeItem(const FWEventItem* iItem) {
   assert("FWEventItemsManager::removeItem not implemented");
}


//
// const member functions
//
FWEventItemsManager::const_iterator FWEventItemsManager::begin() const { return m_items.begin(); }
FWEventItemsManager::const_iterator FWEventItemsManager::end() const { return m_items.end(); }

/** Look up an item by name.
  */
const FWEventItem* FWEventItemsManager::find(const std::string& iName) const {
  for (size_t i = 0, e = m_items.size(); i != e; ++i) {
    const FWEventItem* item = m_items[i];
    if (item && item->name() == iName)
      return item;
  }
  return nullptr;
}

//
// static member functions
//
