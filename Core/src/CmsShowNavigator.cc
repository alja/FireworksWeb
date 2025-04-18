// -*- C++ -*-
//
// Package:     newVersion
// Class  :     CmsShowNavigator
//


// system include files
#include <functional>
#include <iostream>
#include "boost/regex.hpp"
#include "TROOT.h"
#include "TTree.h"
#include "TFile.h"
#include "TError.h"
#include "TGTextEntry.h"
#include "TGNumberEntry.h"
#include "TBranch.h"
#include "TAxis.h"
#include "ROOT/REveManager.hxx"
#include "ROOT/REveScene.hxx"

#include "TApplication.h"
#include "TSystem.h"
#include "TObject.h"

// user include files
#include "DataFormats/FWLite/interface/Event.h"
#include "FireworksWeb/Core/interface/FW2Main.h"

#include "FireworksWeb/Core/interface/CmsShowNavigator.h"
#include "FireworksWeb/Core/interface/FWWebEventItemsManager.h"
#include "FireworksWeb/Core/interface/FWWebGUIEventFilter.h"
#include "FireworksWeb/Core/interface/FW2TEventList.h"
#include "FireworksWeb/Core/interface/FWLiteJobMetadataManager.h"
#include "FireworksWeb/Core/interface/FWConfiguration.h"
#include "FireworksWeb/Core/interface/Context.h"
#include "FireworksWeb/Core/interface/fwLog.h"

#include "FireworksWeb/Core/src/FWTTreeCache.h"
//#include "FireworksWeb/Core/src/json.hpp"
#include "TBase64.h"

//
// constructors and destructor
//
CmsShowNavigator::CmsShowNavigator(FW2Main &main)
    : FWNavigatorBase(main),
      m_currentEvent(0),

      m_filterState(kOff),
      m_filterMode(kOr),

      m_filesNeedUpdate(true),
      m_newFileOnNextEvent(false),

      m_maxNumberOfFilesToChain(1),

      m_main(main)
{
  m_guiFilter = new FWWebGUIEventFilter(this);
  m_guiFilter->IncDenyDestroy();

  // ROOT::Experimental::gEve->GetWorld()->LastChild()->AddElement(m_guiFilter);

  // filterStateChanged_.connect(std::bind(&FWWebGUIEventFilter::updateFilterStateLabel, m_guiFilter, std::placeholders::_1));
}

CmsShowNavigator::~CmsShowNavigator()
 {
   m_guiFilter->DecDenyDestroy();
 }

//
// member functions
//

bool CmsShowNavigator::openFile(const std::string& fileName) {
  fwLog(fwlog::kInfo) << "CmsShowNavigator::openFile [" << fileName << "]" << std::endl;
  FWFileEntry* newFile = nullptr;
  try {
    //newFile = new FWFileEntry(fileName, m_main.getVersionCheck(), m_main.getGlobalTagCheck());
    // AMT TODO add command line options for version check and global tag check
    newFile = new FWFileEntry(fileName, m_main.getVersionCheck(), m_main.getGlobalTagCheck());
  } catch (std::exception& exc) {
    fwLog(fwlog::kError) << "Navigator::openFile ecaught exception FWFileEntry constructor " << exc.what()
                         << std::endl;

    delete newFile;
    return false;
  }

  try {
    // delete all previous files
    while (!m_files.empty()) {
      FWFileEntry* file = m_files.front();
      m_files.pop_front();
      file->closeFile();
      delete file;
    }

    m_files.push_back(newFile);
    setCurrentFile(m_files.begin());

    // set filters
    for (std::list<FWEventSelector*>::iterator i = m_selectors.begin(); i != m_selectors.end(); ++i)
      newFile->filters().push_back(new FWFileEntry::Filter(*i));

    if (m_filterState != kOff)
      updateFileFilters();

    return true;
  } catch (std::exception& exc) {
    fwLog(fwlog::kError) << "Navigator::openFile caught exception " << exc.what() << std::endl;
    return false;
  }
}

bool CmsShowNavigator::appendFile(const std::string& fileName, bool checkFileQueueSize, bool live) {
  fwLog(fwlog::kInfo) << "CmsShowNavigator::appendFile [" << fileName << "]" << std::endl;
  FWFileEntry* newFile = nullptr;
  try {
    newFile = new FWFileEntry(fileName, m_main.getVersionCheck(), m_main.getGlobalTagCheck());
  } catch (std::exception& exc) {
    fwLog(fwlog::kError) << "Navigator::appendFile caught exception FWFileEntry constructor.\n" << exc.what()
                         << std::endl;
    delete newFile;
    return false;
  }

  try {
    if (checkFileQueueSize) {
      int toErase = m_files.size() - (m_maxNumberOfFilesToChain + 1);
      while (toErase > 0) {
        FileQueue_i si = m_files.begin();
        if (m_currentFile == si)
          si++;
        FWFileEntry* file = *si;
        file->closeFile();
        delete file;

        m_files.erase(si);
        --toErase;
      }

      if (m_files.size() > m_maxNumberOfFilesToChain)
        fwLog(fwlog::kWarning) << "  " << m_files.size() << " chained files more than maxNumberOfFilesToChain \n"
                               << m_maxNumberOfFilesToChain << std::endl;
    }

    m_files.push_back(newFile);

    // Needed for proper handling of first registered file when -port option is in effect.
    if (!m_currentFile.isSet())
      setCurrentFile(m_files.begin());

    // set filters
    for (std::list<FWEventSelector*>::iterator i = m_selectors.begin(); i != m_selectors.end(); ++i)
      newFile->filters().push_back(new FWFileEntry::Filter(*i));

    if (m_filterState != kOff)
      updateFileFilters();

  } catch (std::exception& exc) {
    std::cerr << "Navigator::openFile caught exception " << exc.what() << std::endl;
    return false;
  }

  return true;
}

//______________________________________________________________________________

void CmsShowNavigator::setCurrentFile(FileQueue_i fi) {
  if (fwlog::presentLogLevel() == fwlog::kInfo) {
    int cnt = 0;
    for (FileQueue_i i = m_files.begin(); i != m_files.end(); i++) {
      if (i == fi)
        break;
      cnt++;
    }

    fwLog(fwlog::kDebug) << "CmsShowNavigator::setCurrentFile [" << (*fi)->file()->GetName() << "] file idx in chain ["
                         << cnt << "/" << m_files.size() - 1 << "]" << std::endl;
  } else {
    fwLog(fwlog::kInfo) << "Set current file to " << (*fi)->file()->GetName() << ".\n";
  }

  m_currentFile = fi;
  fileChanged_.emit((*m_currentFile)->file());
}

void CmsShowNavigator::goTo(FileQueue_i fi, int event) {
  
  const std::lock_guard<std::mutex> lock(m_mutex);

  if (fi != m_currentFile)
    setCurrentFile(fi);

  if (fwlog::presentLogLevel() == fwlog::kDebug) {
    int total = (*fi)->tree()->GetEntries();
    fwLog(fwlog::kDebug) << "CmsShowNavigator::goTo  current file event [" << event << "/" << total - 1 << "]"
                         << std::endl;

    CpuInfo_t cpuInfo;
    MemInfo_t memInfo;
    ProcInfo_t procInfo;
    gSystem->GetCpuInfo(&cpuInfo, 0);
    gSystem->GetMemInfo(&memInfo);
    gSystem->GetProcInfo(&procInfo);

    time_t curtime;
    time(&curtime);

    fwLog(fwlog::kDebug) << "Current Time: " << ctime(&curtime);
    fwLog(fwlog::kDebug) << "memInfo.fMemUsed \t" << memInfo.fMemUsed << std::endl;
    fwLog(fwlog::kDebug) << "memInfo.fSwapUsed\t" << memInfo.fSwapUsed << std::endl;
    fwLog(fwlog::kDebug) << "procInfo.fMemResident\t" << procInfo.fMemResident << std::endl;
    fwLog(fwlog::kDebug) << "procInfo.fMemVirtual\t" << procInfo.fMemVirtual << std::endl;
    fwLog(fwlog::kDebug) << "cpuInfo.fLoad1m \t" << cpuInfo.fLoad1m << std::endl;
    fwLog(fwlog::kDebug) << "cpuInfo.fLoad5m \t" << cpuInfo.fLoad5m << std::endl;
  }

  if (FWTTreeCache::IsLogging()) {
    printf("FWTTreeCache statistics before going to event %d:\n", event);
    (*m_currentFile)->tree()->PrintCacheStats();  // ("cachedbranches");
  }

  (*m_currentFile)->event()->to(event);
  (*m_currentFile)->tree()->LoadTree(event);
  m_currentEvent = event;

  m_frameTitle = produceFrameTitle();
  newEvent_.emit();
}

void CmsShowNavigator::goToRunEvent(edm::RunNumber_t run, edm::LuminosityBlockNumber_t lumi, edm::EventNumber_t event) {
  for (FileQueue_i file = m_files.begin(); file != m_files.end(); ++file) {
    Long64_t index = (*file)->event()->indexFromEventId(run, lumi, event);
    if (index >= 0) {
      goTo(file, index);
      break;
    }
    else {
      fwLog(fwlog::kError) << "CmsShowNavigator::goToRunEvent can't locate :" << run <<", lumi: " << lumi << ",event " << event ;
    }
  }
}

//______________________________________________________________________________

void CmsShowNavigator::firstEvent() {
  FileQueue_i x = m_files.begin();
  if (m_filterState == kOn) {
    while (x != m_files.end()) {
      if ((*x)->hasSelectedEvents()) {
        goTo(x, (*x)->firstSelectedEvent());
        return;
      }
      ++x;
    }
  } else {
    goTo(x, 0);
  }
}

void CmsShowNavigator::lastEvent() {
  FileQueue_i x = m_files.end();
  if (m_filterState == kOn) {
    while (x != m_files.begin()) {
      --x;
      if ((*x)->hasSelectedEvents()) {
        goTo(x, (*x)->lastSelectedEvent());
        return;
      }
    }
  } else {
    --x;
    goTo(x, (*x)->lastEvent());
  }
}

//______________________________________________________________________________

bool CmsShowNavigator::nextSelectedEvent() {
  int nextEv = (*m_currentFile)->nextSelectedEvent(m_currentEvent);
  if (nextEv > -1) {
    goTo(m_currentFile, nextEv);
    return true;
  } else {
    FileQueue_i i = m_currentFile;
    ++i;
    while (i != m_files.end()) {
      if ((*i)->hasSelectedEvents()) {
        goTo(i, (*i)->firstSelectedEvent());
        return true;
      }
      ++i;
    }
  }

  return false;
}

//______________________________________________________________________________

void CmsShowNavigator::nextEvent() {
  if (m_newFileOnNextEvent) {
    FileQueue_i last = m_files.end();
    --last;
    if (m_filterState == kOn)
      goTo(last, (*last)->firstSelectedEvent());
    else
      goTo(last, 0);

    m_newFileOnNextEvent = false;
    return;
  }

  if (m_filterState == kOn) {
    nextSelectedEvent();
    return;
  } else {
    if (m_currentEvent < (*m_currentFile)->lastEvent()) {
      goTo(m_currentFile, m_currentEvent + 1);
    } else {
      FileQueue_i x = m_currentFile;
      ++x;
      if (x != m_files.end()) {
        goTo(x, 0);
      }
    }
  }
}

//______________________________________________________________________________

bool CmsShowNavigator::previousSelectedEvent() {
  int prevEv = (*m_currentFile)->previousSelectedEvent(m_currentEvent);
  if (prevEv > -1) {
    goTo(m_currentFile, prevEv);
    return true;
  } else {
    FileQueue_i i(m_currentFile);
    i.previous(m_files);
    while (i != m_files.end()) {
      if ((*i)->hasSelectedEvents()) {
        goTo(i, (*i)->lastSelectedEvent());
        return true;
      }
      i.previous(m_files);
    }
  }
  return false;
}

//______________________________________________________________________________

void CmsShowNavigator::previousEvent() {
  if (m_filterState == kOn) {
    previousSelectedEvent();
  } else {
    if (m_currentEvent > 0) {
      goTo(m_currentFile, m_currentEvent - 1);
    } else {
      // last event in previous file
      FileQueue_i x = m_currentFile;
      if (x != m_files.begin()) {
        --x;
        goTo(x, (*x)->lastEvent());
      }
    }
  }
}

//______________________________________________________________________________

void CmsShowNavigator::toggleFilterEnable() {
  // callback

  fwLog(fwlog::kInfo) << "CmsShowNavigator::toggleFilterEnable filters enabled [" << (m_filterState == kOff) << "]"
                      << std::endl;

  if (m_filterState == kOff) {
    m_filterState = kOn;
    updateFileFilters();
  } else {
    m_filterState = kOff;
  }

  filterStateChanged_.emit(m_filterState);
}

void CmsShowNavigator::withdrawFilter() {
  fwLog(fwlog::kInfo) << "CmsShowNavigator::witdrawFilter" << std::endl;
  m_filterState = kWithdrawn;
  filterStateChanged_.emit(m_filterState);
}

void CmsShowNavigator::resumeFilter() {
  fwLog(fwlog::kInfo) << "CmsShowNavigator::resumeFilter" << std::endl;
  m_filterState = kOn;
  filterStateChanged_.emit(m_filterState);
}

void CmsShowNavigator::updateFileFilters() {
    m_filterState = kBusy;
    //m_filter_thr = std::thread{[this] { runFilterThread(); }};
    std::thread{[this] { runFilterThread(); }}.detach();
}

void  CmsShowNavigator::runFilterThread()
{
  std::cout << "run filters in a thread \n";

  // run filters on files
  std::list<FWFileEntry::Filter>::iterator it;
  for (FileQueue_i file = m_files.begin(); file != m_files.end(); ++file) {
    if (m_filesNeedUpdate)
      (*file)->needUpdate();
    (*file)->updateFilters(m_main.getEventItemsManager(), m_filterMode == kOr, m_guiFilter, this);
  }

    ROOT::Experimental::REveManager::ChangeGuard ch;
  // filter processing aborted
  if (m_filterState == kOff)
  {
    fwLog(fwlog::kInfo) << "CmsShowNavigator::updateFileFilters aborted!\n";
    m_guiFilter->StampObjProps();
    return;
  }

  updateSelectorsInfo();
  m_filesNeedUpdate = false;

  int nSelected = getNSelectedEvents();
  if (nSelected) {
    printf("N selected %d\n", nSelected);
    // go to the nearest selected event/file
    bool changeCurrentEvent = !(*m_currentFile)->isEventSelected(m_currentEvent);
    if (changeCurrentEvent) {
      // printf("CmsShow navigator change current event \n");
      if (!nextSelectedEvent())
        previousSelectedEvent();
    }

    //if (m_filterState == kWithdrawn)
   //   resumeFilter();

    m_filterState = kOn;
    postFiltering_.emit(changeCurrentEvent);
  } else {
    withdrawFilter();
  }

  m_guiFilter->StampObjProps();

  if (fwlog::presentLogLevel() == fwlog::kDebug) {
    fwLog(fwlog::kDebug) << "CmsShowNavigator::updateFileFilters selected events over files [" << getNSelectedEvents()
                         << "/" << getNTotalEvents() << "]" << std::endl;
                     
  }
}

//=======================================================================
void CmsShowNavigator::removeFilter(std::list<FWEventSelector*>::iterator si) {
  fwLog(fwlog::kDebug) << "CmsShowNavigator::removeFilter " << (*si)->m_expression << std::endl;

  std::list<FWFileEntry::Filter*>::iterator it;
  for (FileQueue_i file = m_files.begin(); file != m_files.end(); ++file) {
    for (it = (*file)->filters().begin(); it != (*file)->filters().end(); ++it) {
      if ((*it)->m_selector == *si) {
        FWFileEntry::Filter* f = *it;
        delete f;
        (*file)->filters().erase(it);
        break;
      }
    }
  }

  delete *si;
  m_selectors.erase(si);
  m_filesNeedUpdate = true;
}

void CmsShowNavigator::addFilter(const FWEventSelector& ref) {
  fwLog(fwlog::kDebug) << "CmsShowNavigator::addFilter " << ref.m_expression << std::endl;

  FWEventSelector* selector = new FWEventSelector(); // id set automatically
  selector->m_expression  = ref.m_expression;
  selector->m_enabled = ref.m_enabled;

  m_selectors.push_back(selector);

  for (FileQueue_i file = m_files.begin(); file != m_files.end(); ++file) {
    (*file)->filters().push_back(new FWFileEntry::Filter(selector));
  }
  m_filesNeedUpdate = true;
}

void CmsShowNavigator::changeFilter(FWEventSelector* selector, bool updateFilter) {
  fwLog(fwlog::kDebug) << "CmsShowNavigator::changeFilter " << selector->m_expression << std::endl;

  std::list<FWFileEntry::Filter*>::iterator it;
  for (FileQueue_i file = m_files.begin(); file != m_files.end(); ++file) {
    for (it = (*file)->filters().begin(); it != (*file)->filters().end(); ++it) {
      if ((*it)->m_selector == selector) {
        if (updateFilter)
          (*it)->m_needsUpdate = true;
        (*it)->m_selector->m_expression = selector->m_expression;
        break;
      }
    }
  }
  m_filesNeedUpdate = true;
}

//______________________________________________________________________________
// helpers for gui state

bool CmsShowNavigator::isFirstEvent() {
  if (m_filterState == kOn) {
    FileQueue_i firstSelectedFile;
    for (FileQueue_i file = m_files.begin(); file != m_files.end(); ++file) {
      if ((*file)->hasSelectedEvents()) {
        firstSelectedFile = file;
        break;
      }
    }

    if (firstSelectedFile == m_currentFile)
      return (*m_currentFile)->firstSelectedEvent() == m_currentEvent;
  } else {
    if (m_currentFile == m_files.begin()) {
      return m_currentEvent == 0;
    }
  }
  return false;
}

bool CmsShowNavigator::isLastEvent() {
  if (m_filterState == kOn) {
    FileQueue_i lastSelectedFile;
    for (FileQueue_i file = m_files.begin(); file != m_files.end(); ++file) {
      if ((*file)->hasSelectedEvents())
        lastSelectedFile = file;
    }
    if (lastSelectedFile == m_currentFile)
      return (*m_currentFile)->lastSelectedEvent() == m_currentEvent;
  } else {
    FileQueue_i lastFile = m_files.end();
    --lastFile;
    if (m_currentFile == lastFile) {
      return (*m_currentFile)->lastEvent() == m_currentEvent;
    }
  }
  return false;
}

//______________________________________________________________________________
void CmsShowNavigator::updateSelectorsInfo() {
  // reset
  std::list<FWEventSelector*>::const_iterator sel = m_selectors.begin();
  while (sel != m_selectors.end()) {
    (*sel)->m_selected = 0;
    (*sel)->m_updated = true;
    ++sel;
  }

  // loop file filters
  std::list<FWFileEntry::Filter*>::iterator i;
  for (FileQueue_i file = m_files.begin(); file != m_files.end(); ++file) {
    std::list<FWFileEntry::Filter*>& filters = (*file)->filters();
    for (i = filters.begin(); i != filters.end(); ++i) {
      if ((*i)->m_eventList) {
        (*i)->m_selector->m_selected += (*i)->m_eventList->GetN();
      }

      if ((*i)->m_needsUpdate)
        (*i)->m_selector->m_updated = false;
    }
  }
  /*
  if (m_guiFilter) {
    std::list<FWGUIEventSelector*>::const_iterator gs = m_guiFilter->guiSelectors().begin();
    while (gs != m_guiFilter->guiSelectors().end()) {
      (*gs)->updateNEvents();
      ++gs;
    }
  }
  */
}

int CmsShowNavigator::getNSelectedEvents() {
  int sum = 0;
  for (FileQueue_i file = m_files.begin(); file != m_files.end(); ++file) {
    if ((*file)->globalSelection())
      sum += (*file)->globalSelection()->GetN();
  }
  return sum;
}

int CmsShowNavigator::getNTotalEvents() {
  int sum = 0;
  for (FileQueue_i file = m_files.begin(); file != m_files.end(); ++file) {
    sum += (*file)->tree()->GetEntries();
  }

  return sum;
}

const char* CmsShowNavigator::filterStatusMessage() {
  if (m_filterState == kOn)
    return Form("%d events are selected from %d.", getNSelectedEvents(), getNTotalEvents());
  else if (m_filterState == kOff)
    return "Filtering is OFF.";
  else
    return "Filtering is disabled.";
}

bool CmsShowNavigator::canEditFiltersExternally() {
  bool haveActiveFilters = false;
  for (FileQueue_i file = m_files.begin(); file != m_files.end(); ++file) {
    if ((*file)->hasActiveFilters()) {
      haveActiveFilters = true;
      break;
    }
  }

  bool btnEnabled = haveActiveFilters;

//  if (m_guiFilter && m_guiFilter->isOpen())
//    btnEnabled = false;

  return btnEnabled;
}

void CmsShowNavigator::editFiltersExternally() { editFiltersExternally_.emit(canEditFiltersExternally()); }

void CmsShowNavigator::showEventFilterGUI(const TGWindow* p) {
  /*
  if (m_guiFilter->IsMapped()) {
    m_guiFilter->CloseWindow();
  } else {
    m_guiFilter->show(&m_selectors, m_filterMode, m_filterState);
    editFiltersExternally_.emit(canEditFiltersExternally());
  }
  */
}

//______________________________________________________________________________

void CmsShowNavigator::setFrom(const FWConfiguration& iFrom) {
  m_filesNeedUpdate = true;

  EFilterState oldFilterState = m_filterState;

  m_selectors.clear();

  // selectors
  const FWConfiguration* sConf = iFrom.valueForKey("EventFilters");

  if (sConf) {
    const FWConfiguration::KeyValues* keyVals = sConf->keyValues();
    if (keyVals) {
      for (FWConfiguration::KeyValuesIt it = keyVals->begin(); it != keyVals->end(); ++it) {
        const FWConfiguration& conf = (*it).second;
        FWEventSelector* selector = new FWEventSelector();
        selector->m_expression = conf.valueForKey("expression")->value();
        selector->m_description = conf.valueForKey("comment")->value();
        selector->m_enabled = atoi(conf.valueForKey("enabled")->value().c_str());
        if (conf.valueForKey("triggerProcess"))
          selector->m_triggerProcess = conf.valueForKey("triggerProcess")->value();
        m_selectors.push_back(selector);
      }
    }
  } 

  // filter mode
  {
    const FWConfiguration* value = iFrom.valueForKey("EventFilter_mode");
    if (value) {
      std::istringstream s(value->value());
      s >> m_filterMode;
    }
  }

  // filter on
  {
    const FWConfiguration* value = iFrom.valueForKey("EventFilter_enabled");
    std::istringstream s(value->value());
    int x;
    s >> x;
    m_filterState = x ? kOn : kOff;
  }

/*
  // redesplay new filters in event filter dialog if already mapped
  if (m_guiFilter) {
    m_guiFilter->reset();
    if (m_guiFilter->IsMapped())
      m_guiFilter->show(&m_selectors, m_filterMode, m_filterState);
  }
*/
  if (!m_files.empty()) {
    // change filters in existing files
    for (FileQueue_i file = m_files.begin(); file != m_files.end(); ++file) {
      (*file)->filters().clear();
      for (std::list<FWEventSelector*>::iterator i = m_selectors.begin(); i != m_selectors.end(); ++i)
        (*file)->filters().push_back(new FWFileEntry::Filter(*i));
    }

    // run new filters if enabled, else just reset
    // AMT misleading function name
    // filter call in the same thread
    if (m_filterState == kOn)
       runFilterThread();
    //  updateFileFilters();
    //else
      //postFiltering_.emit(true);
  }
  // update CmsShowMainFrame checkBoxIcon and button text
  if (oldFilterState != m_filterState)
    filterStateChanged_.emit(m_filterState);
}

void CmsShowNavigator::addTo(FWConfiguration& iTo) const {
  // selectors
  FWConfiguration tmp;
  int cnt = 0;
  for (std::list<FWEventSelector*>::const_iterator sel = m_selectors.begin(); sel != m_selectors.end(); ++sel) {
    FWConfiguration filter;

    filter.addKeyValue("expression", FWConfiguration((*sel)->m_expression));
    filter.addKeyValue("enabled", FWConfiguration((*sel)->m_enabled ? "1" : "0"));
    filter.addKeyValue("comment", FWConfiguration((*sel)->m_description));

    if (!(*sel)->m_triggerProcess.empty())
      filter.addKeyValue("triggerProcess", FWConfiguration((*sel)->m_triggerProcess));

    tmp.addKeyValue(Form("------Filter[%d]------", cnt), filter, true);
    ++cnt;
  }

  iTo.addKeyValue("EventFilters", tmp, true);

  // mode
  {
    std::stringstream s;
    s << m_filterMode;
    iTo.addKeyValue("EventFilter_mode", s.str());
  }

  // enabled
  iTo.addKeyValue("EventFilter_enabled", FWConfiguration(m_filterState == kOn ? "1" : "0"));
}

std::vector<std::string>& CmsShowNavigator::getProcessList() const {
  return m_main.getMetadataManager()->processNamesInJob();
}

const edm::EventBase* CmsShowNavigator::getCurrentEvent() const {
  return m_currentFile.isSet() ? (*m_currentFile)->event() : nullptr;
}

const char* CmsShowNavigator::frameTitle() {
  return m_frameTitle.c_str();
}


const char* CmsShowNavigator::produceFrameTitle() {
  if (m_files.empty())
    return "";

  int nf = 0;
  for (FileQueue_t::const_iterator i = m_files.begin(); i != m_files.end(); i++) {
    if (i == m_currentFile)
      break;
    nf++;
  }

  TString name = (*m_currentFile)->file()->GetName();
  int l = name.Last('/');
  if (l != kNPOS)
    name.Remove(0, l + 1);

  if (m_files.size() < 2)
  {
    return Form("%s [%d/%d]",
                name.Data(),
                m_currentEvent + 1,
                (*m_currentFile)->lastEvent() + 1);
  }
  else
  {
    return Form("%s [%d/%d], file [%d/%d]",
                name.Data(),
                m_currentEvent + 1,
                (*m_currentFile)->lastEvent() + 1,
                nf + 1,
                (int)m_files.size());
  }
}

std::string CmsShowNavigator::getCurrentGlobalTag() {
  std::string res;
  if (!m_files.empty()) {
    res = (*m_currentFile)->getGlobalTag();
  }
  return res;
}
