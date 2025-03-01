// -*- C++ -*-
#ifndef FireworksWeb_Core_CmsShowNavigator_h
#define FireworksWeb_Core_CmsShowNavigator_h
//
// Package:     newVersion
// Class  :     CmsShowNavigator
//

// system include files
#include <string>
#include <mutex>
#include <sigc++/sigc++.h>

// user include files
#include "FireworksWeb/Core/interface/FWNavigatorBase.h"
#include "FireworksWeb/Core/interface/FWEventSelector.h"
#include "FireworksWeb/Core/interface/FWConfigurable.h"
#include "FireworksWeb/Core/interface/FWFileEntry.h"

#include "DataFormats/FWLite/interface/Event.h"

#include "TEventList.h"

// forward declarations
class TEventList;
class CSGAction;
class FW2Main;
class TFile;
class TGWindow;
class FWWebGUIEventFilter;

namespace edm
{
  class EventBase;
  class EventID;
} // namespace edm

class CmsShowNavigator : public FWNavigatorBase
{
  friend class FWWebGUIEventFilter;

public:
  enum EFilterState
  {
    kOff,
    kOn,
    kWithdrawn,
    kBusy
  };
  enum EFilterMode
  {
    kOr = 1,
    kAnd = 2
  };

private:
  typedef std::list<FWFileEntry *> FQBase_t;
  typedef FQBase_t::iterator FQBase_i;

  struct FileQueue_t : public FQBase_t
  {
    struct iterator : public FQBase_i
    {
    private:
      bool m_isSet;

    public:
      iterator() : m_isSet(false) {}
      iterator(FQBase_i i) : FQBase_i(i), m_isSet(true) {}

      bool isSet() const { return m_isSet; }

      iterator &previous(FileQueue_t &cont)
      {
        // Go back one element, set to end() when falling off the end.
        if (*this == cont.begin())
          *this = cont.end();
        else
          FQBase_i::operator--();
        return *this;
      }
    };

    FileQueue_t() : FQBase_t() {}

    iterator begin() { return iterator(FQBase_t::begin()); }
    iterator end() { return iterator(FQBase_t::end()); }
  };

  typedef FileQueue_t::iterator FileQueue_i;

  const char* produceFrameTitle();
public:
  CmsShowNavigator(FW2Main &);
  ~CmsShowNavigator() override;

  //configuration management interface
  void addTo(FWConfiguration &) const override;
  void setFrom(const FWConfiguration &) override;

  Int_t realEntry(Int_t rawEntry);
  bool openFile(const std::string &fileName);
  bool appendFile(const std::string &fileName, bool checkFileQueueSize, bool live);

  void nextEvent() override;
  void previousEvent() override;
  bool nextSelectedEvent() override;
  bool previousSelectedEvent() override;
  void firstEvent() override;
  void lastEvent() override;
  void goToRunEvent(edm::RunNumber_t, edm::LuminosityBlockNumber_t, edm::EventNumber_t) override;
  void goTo(FileQueue_i fi, int event);

  void eventFilterEnableCallback(Bool_t);
  void filterEvents();
  void filterEventsAndReset();

  void setMaxNumberOfFilesToChain(unsigned int i) { m_maxNumberOfFilesToChain = i; }

  bool isLastEvent() override;
  bool isFirstEvent() override;

  void showEventFilterGUI(const TGWindow *p);
  // void applyFiltersFromGUI(const char*);
  void toggleFilterEnable();
  void withdrawFilter();
  void resumeFilter();

  const edm::EventBase *getCurrentEvent() const override;

  const char *frameTitle();
  const char *filterStatusMessage();
  std::string getCurrentGlobalTag();

  int getNSelectedEvents() override;
  int getNTotalEvents() override;
  bool canEditFiltersExternally();
  bool filesNeedUpdate() const { return m_filesNeedUpdate; }
  int getFilterState() const { return m_filterState; }

  void editFiltersExternally();

  void activateNewFileOnNextEvent() { m_newFileOnNextEvent = true; }
  void resetNewFileOnNextEvent() { m_newFileOnNextEvent = false; }

  FWWebGUIEventFilter *getGUIFilter() { return m_guiFilter; }

  std::vector<std::string> &getProcessList() const;

  sigc::signal<void(const TFile *)> fileChanged_;
  sigc::signal<void()> preFiltering_;
  sigc::signal<void(bool)> postFiltering_;
  sigc::signal<void(bool)> editFiltersExternally_;
  sigc::signal<void(int)> filterStateChanged_;

protected:
  CmsShowNavigator(const CmsShowNavigator &);                  // stop default
  const CmsShowNavigator &operator=(const CmsShowNavigator &); // stop default

  void setCurrentFile(FileQueue_i);
  void updateFileFilters();
  void runFilterThread();
  void updateSelectorsInfo();

  void removeFilter(std::list<FWEventSelector *>::iterator);
  void addFilter(const FWEventSelector &);
  void changeFilter(FWEventSelector *, bool filterNeedUpdate);

  void newFile(FileQueue_i);

  // ---------- member data --------------------------------

  std::list<FWEventSelector *> m_selectors;
  FileQueue_t m_files;
  FileQueue_i m_currentFile;
  int m_currentEvent;

  EFilterState m_filterState;
  int m_filterMode;
  bool m_filesNeedUpdate;
  bool m_newFileOnNextEvent;

  std::string m_frameTitle; // cached

  unsigned int m_maxNumberOfFilesToChain;
  // entry is an event index nubmer which runs from 0 to
  // #events or #selected_events depending on if we filter
  // events or not
  FW2Main &m_main;
  FWWebGUIEventFilter *m_guiFilter;

  // file navigation can be executed from diffrent threads
  std::mutex m_mutex;
};

#endif
