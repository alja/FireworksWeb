// -*- C++ -*-
#ifndef FireworksWeb_Core_FWFileEntry_h
#define FireworksWeb_Core_FWFileEntry_h
//
// Package:     Core
// Class  :     FWFileEntry
//

// system include files
#include <string>
#include <sigc++/sigc++.h>

#include "TTree.h"

// user include files
// MT -- to get auxBranch
#include "DataFormats/FWLite/interface/Event.h"
#include "FireworksWeb/Core/interface/FWEventSelector.h"
#include "FireworksWeb/Core/interface/FW2TEventList.h"
#include "FireworksWeb/Core/interface/FWConfigurable.h"

// forward declarations
class FWWebEventItem;
class FW2TEventList;
class FWTTreeCache;
class CSGAction;
class CmsShowMain;
class TFile;
class TGWindow;
class FWWebEventItemsManager;
class FWWebGUIEventFilter;
class CmsShowNavigator;

namespace internal
{
  class FireworksProductGetter;
}

namespace edm {
  class EventID;
  //class ProcessHistoryVector;
}

class FWFileEntry {
public:
  struct Filter {
    FW2TEventList* m_eventList;
    FWEventSelector* m_selector;  // owned by navigator
    bool m_needsUpdate;

    Filter(FWEventSelector* s) : m_eventList(nullptr), m_selector(s), m_needsUpdate(true) {}
    ~Filter() { delete m_eventList; }

    bool hasSelectedEvents() { return m_eventList && m_eventList->GetN(); }
  };

  FWFileEntry(const std::string& name, bool checkVersion, bool checkGlobalTag);
  virtual ~FWFileEntry();

  TFile* file() { return m_file; }
  fwlite::Event* event() { return m_event; }
  TTree* tree() { return m_eventTree; }
  FW2TEventList* globalSelection() { return m_globalEventList; }
  FWTTreeCache* fwTreeCache();

  std::list<Filter*>& filters() { return m_filterEntries; }

  const std::string& getGlobalTag() const { return m_globalTag; }

  void openFile(bool, bool);
  void closeFile();

  bool isEventSelected(int event);

  bool hasSelectedEvents();

  bool hasActiveFilters();

  int firstSelectedEvent();
  int lastSelectedEvent();

  int lastEvent() { return m_eventTree->GetEntries() - 1; }

  int nextSelectedEvent(int event);
  int previousSelectedEvent(int event);

  void needUpdate() { m_needUpdate = true; }
  void updateFilters(const FWWebEventItemsManager* eiMng, bool isOR, FWWebGUIEventFilter*, const CmsShowNavigator*);

  // CallIns from FWWebEventItemsManager for tree-cache add/remove branch
  void NewEventItemCallIn(const FWWebEventItem* it);
  void RemovingEventItemCallIn(const FWWebEventItem* it);

private:
  FWFileEntry(const FWFileEntry&) = delete;                   // stop default
  const FWFileEntry& operator=(const FWFileEntry&) = delete;  // stop default

  void runCollectionFilter(Filter* fe, const FWWebEventItemsManager* eiMng, FWWebGUIEventFilter*, const CmsShowNavigator*);
  bool runHLTFilter(Filter* filter, FWWebGUIEventFilter*);

  std::string getBranchName(const FWWebEventItem* it) const;

  // used from this constructor
  void setGlobalTag(edm::ProcessHistoryVector&);
  void checkDataVersion(edm::ProcessHistoryVector&);

  std::string m_name;
  TFile* m_file;
  TTree* m_eventTree;
  fwlite::Event* m_event;

  // filtering
  TFile* m_filterFile{nullptr};
  TTree* m_filterEventTree{nullptr};
  fwlite::Event* m_filterEvent{nullptr};
  std::shared_ptr<internal::FireworksProductGetter> m_productGetter;

  bool m_needUpdate;  // To be set in navigator::filterChanged/Added, newFile
  std::string m_globalTag;

  std::list<Filter*> m_filterEntries;
  FW2TEventList* m_globalEventList;
};
#endif
