
#include <boost/regex.hpp>
#include <signal.h>

#include "TFile.h"
#include "TEveTreeTools.h"
#include "TError.h"
#include "TMath.h"
#include "TEnv.h"
#include "TROOT.h"

#include <ROOT/REveTreeTools.hxx>
#include <ROOT/REveManager.hxx>

#include "DataFormats/FWLite/interface/Handle.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/Provenance/interface/ProcessConfiguration.h"
#include "DataFormats/Provenance/interface/ProcessHistory.h"
#include "DataFormats/Provenance/interface/ReleaseVersion.h"
#include "DataFormats/Provenance/interface/ParameterSetBlob.h"
#include "DataFormats/Provenance/interface/ParameterSetID.h"
#include "DataFormats/Provenance/interface/ProcessHistoryRegistry.h"

#include "DataFormats/Common/interface/EDProductGetter.h"
#include "FWCore/FWLite/interface/setRefStreamer.h"

#include "FWCore/Utilities/interface/WrappedClassName.h"
#include "FWCore/Common/interface/TriggerNames.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/Registry.h"

#include "FireworksWeb/Core/interface/FWWebEventItem.h"
#include "FireworksWeb/Core/interface/FWFileEntry.h"
#include "FireworksWeb/Core/interface/FWWebEventItemsManager.h"
#include "FireworksWeb/Core/interface/fwLog.h"
#include "FireworksWeb/Core/interface/fwPaths.h"
#include "FireworksWeb/Core/interface/FWWebGUIEventFilter.h"
#include "FireworksWeb/Core/interface/CmsShowNavigator.h"

#include "FireworksWeb/Core/src/FWTTreeCache.h"

#include <functional>
// #define private protected
#include "FWCore/FWLite/interface/BareRootProductGetterBase.h"


namespace internal {
class FireworksProductGetter : public BareRootProductGetterBase
{
private:
  TFile* m_file{nullptr};

public:
  FireworksProductGetter(TFile* f) : m_file(f) {};
  ~FireworksProductGetter() override {};

private:
  TFile* currentFile() const override {
      if (nullptr == m_file) {
        throw cms::Exception("FileNotFound") << "unable to find the TFile '" << m_file << "'\n";
      }
      return m_file;
  }
};
}



class BareRootProductGetter : public BareRootProductGetterBase {
public:
  BareRootProductGetter() = default;

private:
  TFile* currentFile() const override;
};

//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------

FWFileEntry::FWFileEntry(const std::string& name, bool checkVersion, bool checkGT)
    : m_name(name),
      m_file(nullptr),
      m_eventTree(nullptr),
      m_event(nullptr),
      m_needUpdate(true),
      m_globalTag("gt_undef"),
      m_globalEventList(nullptr)
      {
  openFile(checkVersion, checkGT);
}

FWFileEntry::~FWFileEntry() {
  for (std::list<Filter*>::iterator i = m_filterEntries.begin(); i != m_filterEntries.end(); ++i)
    delete (*i)->m_eventList;

  delete m_globalEventList;
}
//------------------------------------------
void FWFileEntry::openFile(bool checkVersion, bool checkGlobalTag) {
  gErrorIgnoreLevel = 3000;  // suppress warnings about missing dictionaries

  TFile* newFile = TFile::Open(m_name.c_str());

  if (newFile == nullptr || newFile->IsZombie() || !newFile->Get("Events")) {
    //  std::cout << "Invalid file. Ignored." << std::endl;
    // return false;
    throw std::runtime_error("Invalid file. Ignored.");
  }

  m_file = newFile;

  gErrorIgnoreLevel = -1;

  // check CMSSW relese version for compatibility
  TTree* metaData = dynamic_cast<TTree*>(m_file->Get("MetaData"));
  if (metaData == nullptr) {
    throw std::runtime_error("Cannot find TTree 'MetaData' in the data file");
  }
  TBranch* b = metaData->GetBranch("ProcessHistory");
  edm::ProcessHistoryVector phv_;
  edm::ProcessHistoryVector* pPhv = &phv_;
  metaData->SetBranchAddress("ProcessHistory", &pPhv);
  b->GetEntry(0);

  // read first global tag for auto detect of geomtery version
  if (checkGlobalTag) {
    setGlobalTag(phv_);
  }

  // test compatibility of data with CMSSW
  b->SetAddress(nullptr); // AMT ???
  if (checkVersion) {
    checkDataVersion(phv_);
  }

  m_eventTree = dynamic_cast<TTree*>(m_file->Get("Events"));

  if (m_eventTree == nullptr) {
    throw std::runtime_error("Cannot find TTree 'Events' in the data file");
  }

  // load event, set DataGetterHelper callback for branch access

  // Initialize caching, this helps also in the case of local file.
  if (FWTTreeCache::IsLogging())
    printf("FWFileEntry::openFile enabling FWTTreeCache for file class '%s'.", m_file->ClassName());

  auto tc = new FWTTreeCache(m_eventTree, FWTTreeCache::GetDefaultCacheSize());
  m_file->SetCacheRead(tc, m_eventTree);
  tc->SetEnablePrefetching(FWTTreeCache::IsPrefetching());
  tc->SetLearnEntries(20);
  tc->SetLearnPrefill(TTreeCache::kAllBranches);
  tc->StartLearningPhase();

  try {
    m_event = new fwlite::Event(m_file, false, [tc](TBranch const& b) { tc->BranchAccessCallIn(&b); });
  }
  catch (std::exception& e)
  {
     std::string msg = m_name + " is not a valid edm data file.\nFailed to create a fwlite::Event with error: ";
     msg += e.what();
     throw std::runtime_error(msg);
  }
  // Connect to collection add/remove signals
  // FWWebEventItemsManager* eiMng = (FWWebEventItemsManager*)FWGUIManager::getGUIManager()->getContext()->eventItemsManager();
  // eiMng->newItem_.connect(std::bind(&FWFileEntry::NewEventItemCallIn, this, std::placeholders::_1));
  // eiMng->removingItem_.connect(std::bind(&FWFileEntry::RemovingEventItemCallIn, this, std::placeholders::_1));
  // no need to connect to goingToClearItems_ ... individual removes are emitted.

  if (m_event->size() == 0)
    throw std::runtime_error("fwlite::Event size == 0");

}

//---------------------------------------------

void FWFileEntry::checkDataVersion(edm::ProcessHistoryVector& phv_)
{
  const edm::ProcessConfiguration* dd = nullptr;
  int latestVersion = 0;
  int currentVersionArr[] = {0, 0, 0};
  for (auto const& processHistory : phv_) {
    for (auto const& processConfiguration : processHistory) {
      // std::cout << processConfiguration.releaseVersion() << "  " << processConfiguration.processName() << std::endl;
      TString dcv = processConfiguration.releaseVersion();
      fireworks::getDecomposedVersion(dcv, currentVersionArr);
      int nvv = currentVersionArr[0] * 100 + currentVersionArr[1] * 10 + currentVersionArr[2];
      if (nvv > latestVersion) {
        latestVersion = nvv;
        dd = &processConfiguration;
      }
    }
  }

  if (latestVersion) {
    fwLog(fwlog::kInfo) << "Checking process history. " << m_name.c_str() << " latest process \"" << dd->processName()
                        << "\", version " << dd->releaseVersion() << std::endl;

    // b->SetAddress(nullptr);
    TString v = dd->releaseVersion();
    if (!fireworks::acceptDataFormatsVersion(v)) {
      int* di = (fireworks::supportedDataFormatsVersion());
      TString msg = Form(
          "incompatible data: Process version does not mactch major data formats version. File produced with %s. "
          "Data formats version \"CMSSW_%d_%d_%d\".\n",
          dd->releaseVersion().c_str(),
          di[0],
          di[1],
          di[2]);
      msg += "Use --no-version-check option if you still want to view the file.\n";
      throw std::runtime_error(msg.Data());
    }
  } else {
    TString msg = "No process history available\n";
    msg += "Use --no-version-check option if you still want to view the file.\n";
    throw std::runtime_error(msg.Data());
  }
}

//---------------------------------------------

void FWFileEntry::setGlobalTag(edm::ProcessHistoryVector& phv_)
{
  typedef std::map<edm::ParameterSetID, edm::ParameterSetBlob> ParameterSetMap;
  ParameterSetMap psm_;
  TTree* psetTree = dynamic_cast<TTree*>(m_file->Get("ParameterSets"));
  typedef std::pair<edm::ParameterSetID, edm::ParameterSetBlob> IdToBlobs;
  IdToBlobs idToBlob;
  IdToBlobs* pIdToBlob = &idToBlob;
  psetTree->SetBranchAddress("IdToParameterSetsBlobs", &pIdToBlob);
  for (long long i = 0; i != psetTree->GetEntries(); ++i) {
    psetTree->GetEntry(i);
    psm_.insert(idToBlob);
  }

  edm::pset::Registry& psetRegistry = *edm::pset::Registry::instance();
  for (auto const& item : psm_) {
    edm::ParameterSet pset(item.second.pset());
    pset.setID(item.first);
    psetRegistry.insertMapped(pset);
  }

  std::map<edm::ProcessConfigurationID, unsigned int> simpleIDs;
  m_globalTag = "";
  for (auto const& ph : phv_) {
    for (auto const& pc : ph) {
      unsigned int id = simpleIDs[pc.id()];
      if (0 == id) {
        id = 1;
        simpleIDs[pc.id()] = id;
      }
      ParameterSetMap::const_iterator itFind = psm_.find(pc.parameterSetID());
      if (itFind == psm_.end()) {
        std::cout << "No ParameterSetID for " << pc.parameterSetID() << std::endl;
        fwLog(fwlog::kInfo) << "FWFileEntry::openFile no ParameterSetID for " << pc.parameterSetID() << std::endl;
      } else {
        edm::ParameterSet processConfig(itFind->second.pset());
        std::vector<std::string> sourceStrings, moduleStrings;
        std::vector<std::string> sources = processConfig.getParameter<std::vector<std::string>>("@all_essources");
        for (auto& itM : sources) {
          edm::ParameterSet const& pset = processConfig.getParameterSet(itM);
          std::string name(pset.getParameter<std::string>("@module_label"));
          if (name.empty()) {
            name = pset.getParameter<std::string>("@module_type");
          }
          if (name != "GlobalTag")
            continue;

          for (auto const& item : pset.tbl()) {
            if (item.first == "globaltag") {
              m_globalTag = item.second.getString();
              goto gtEnd;
            }
          }
        }
      }
    }
  }

  gtEnd:
    fwLog(fwlog::kDebug) << "FWFileEntry::openFile detected global tag " << m_globalTag << "\n";
}

//----------------------------------------------------



void FWFileEntry::closeFile()
{
  if (m_file)
  {
    delete m_file->GetCacheRead(m_eventTree);
    m_file->Close();
    delete m_file;
    delete m_event;
  }

  if (m_filterFile)
  {
    delete m_filterFile->GetCacheRead(m_filterEventTree);
    m_filterFile->Close();
    delete m_filterFile;
    delete m_filterEvent;
  }
}

//______________________________________________________________________________

bool FWFileEntry::isEventSelected(int tree_entry) {
  int idx = m_globalEventList->GetIndex(tree_entry);
  return idx >= 0;
}

bool FWFileEntry::hasSelectedEvents() { return m_globalEventList->GetN() > 0; }

int FWFileEntry::firstSelectedEvent() {
  if (m_globalEventList->GetN() > 0) {
    return m_globalEventList->GetEntry(0);
  } else {
    return -1;
  }
}

int FWFileEntry::lastSelectedEvent() {
  if (m_globalEventList->GetN() > 0)
    return m_globalEventList->GetEntry(m_globalEventList->GetN() - 1);
  else
    return -1;
}

int FWFileEntry::nextSelectedEvent(int tree_entry) {
  // Find next selected event after the current one.
  // This returns the index in the selected event list.
  // If none exists -1 is returned.

  const Long64_t* list = m_globalEventList->GetList();
  Long64_t val = tree_entry;
  Long64_t idx = TMath::BinarySearch(m_globalEventList->GetN(), list, val);
  ++idx;
  if (idx >= m_globalEventList->GetN() || idx < 0)
    return -1;
  return list[idx];
}

int FWFileEntry::previousSelectedEvent(int tree_entry) {
  // Find first selected event before current one.
  // This returns the index in the selected event list.
  // If none exists -1 is returned.

  const Long64_t* list = m_globalEventList->GetList();
  Long64_t val = tree_entry;
  Long64_t idx = TMath::BinarySearch(m_globalEventList->GetN(), list, val);
  if (list[idx] == val)
    --idx;
  if (idx >= 0)
    return list[idx];
  else
    return -1;
}

//______________________________________________________________________________
bool FWFileEntry::hasActiveFilters() {
  for (std::list<Filter*>::iterator it = m_filterEntries.begin(); it != m_filterEntries.end(); ++it) {
    if ((*it)->m_selector->m_enabled)
      return true;
  }

  return false;
}

//______________________________________________________________________________
void FWFileEntry::updateFilters(const FWWebEventItemsManager *eiMng, bool globalOR,
                                FWWebGUIEventFilter *gui, const CmsShowNavigator *navigator)
{
  if (!m_needUpdate)
    return;

  if (!m_filterEventTree)
  {
    // std::chrono::time_point<std::chrono::system_clock> tf0 = std::chrono::system_clock::now();
    m_filterFile = TFile::Open(m_file->GetName());
    m_filterEventTree = dynamic_cast<TTree *>(m_filterFile->Get("Events"));
    auto tc = new FWTTreeCache(m_filterEventTree, FWTTreeCache::GetDefaultCacheSize());
    m_filterFile->SetCacheRead(tc, m_filterEventTree);
    // std::chrono::time_point<std::chrono::system_clock> tf1 = std::chrono::system_clock::now();

    tc->SetEnablePrefetching(FWTTreeCache::IsPrefetching());
    tc->SetLearnEntries(20);
    tc->SetLearnPrefill(TTreeCache::kAllBranches);
    tc->StartLearningPhase();

    m_filterEvent = new fwlite::Event(m_filterFile, false, [tc](TBranch const &b)
                                      { tc->BranchAccessCallIn(&b); });
  }


  if (m_globalEventList)
    m_globalEventList->Reset();
  else
    m_globalEventList = new FW2TEventList;

  {
    internal::FireworksProductGetter productGetter(m_filterFile);
    fwlite::GetterOperate op(&productGetter);
    for (std::list<Filter *>::iterator it = m_filterEntries.begin(); it != m_filterEntries.end(); ++it)
    {
      if ((*it)->m_selector->m_enabled && (*it)->m_needsUpdate)
      {
        if ((*it)->m_selector->m_triggerProcess.empty())
        {
          runCollectionFilter(*it, eiMng, gui, navigator);
        }
        else
        {
          runHLTFilter(*it, gui);
        }
      }
      // Need to re-check if enabled after filtering as it can be set to false
      // in runFilter().
      if ((*it)->m_selector->m_enabled)
      {
        if ((*it)->hasSelectedEvents())
        {
          if (globalOR || m_globalEventList->GetN() == 0)
          {
            m_globalEventList->Add((*it)->m_eventList);
          }
          else
          {
            m_globalEventList->Intersect((*it)->m_eventList);
          }
        }
        else if (!globalOR)
        {
          m_globalEventList->Reset();
          break;
        }
      }
    }
  }

  fwLog(fwlog::kDebug) << "FWFileEntry::updateFilters in [" << m_file->GetName() << "]  global selection ["
                       << m_globalEventList->GetN() << "/" << m_eventTree->GetEntries() << "]" << std::endl;

  m_needUpdate = false;
}

//_____________________________________________________________________________
void FWFileEntry::runCollectionFilter(Filter* filter, const FWWebEventItemsManager* eiMng, 
                                      FWWebGUIEventFilter* gui, const CmsShowNavigator* navigator) {
  // parse selection for known Fireworks expressions
  std::string interpretedSelection = filter->m_selector->m_expression;
  // list of branch names to be added to tree-cache
  std::vector<std::string> branch_names;

  for (FWWebEventItemsManager::const_iterator i = eiMng->begin(), end = eiMng->end(); i != end; ++i)
  {
    FWWebEventItem *item = *i;
    if (item == nullptr)
      continue;

    boost::regex re(std::string("\\$") + (*i)->name() + "(?=\\s|\\.|@)");

    if (boost::regex_search(interpretedSelection, re))
    {
      const edm::TypeWithDict elementType(const_cast<TClass *>(item->type()));
      const edm::TypeWithDict wrapperType = edm::TypeWithDict::byName(edm::wrappedClassName(elementType.name()));
      std::string fullBranchName = m_filterEvent->getBranchNameFor(wrapperType.typeInfo(),
                                                             item->moduleLabel().c_str(),
                                                             item->productInstanceLabel().c_str(),
                                                             item->processName().c_str());

      interpretedSelection = boost::regex_replace(interpretedSelection, re, fullBranchName + "obj");

      branch_names.push_back(fullBranchName);
    }
  }

  std::size_t found = interpretedSelection.find('$');
  if (found != std::string::npos) {
    fwLog(fwlog::kError) << "FWFileEntry::RunFilter invalid expression " << interpretedSelection << std::endl;
    filter->m_needsUpdate = false;
    return;
  }

  if (filter->m_eventList)
    filter->m_eventList->Reset();
  else
    filter->m_eventList = new FW2TEventList;

  fwLog(fwlog::kInfo) << "FWFileEntry::runFilter Running filter " << interpretedSelection << "' "
                      << "for file '" << m_file->GetName() << "'.\n";

  ROOT::Experimental::REveSelectorToEventList stoelist(filter->m_eventList, interpretedSelection.c_str());
  
  try
  {
    int Ntotal = m_filterEventTree->GetEntries();
    int step0 = TMath::Min(1000, int(Ntotal*0.1));
    step0 = TMath::Min(step0, Ntotal);
    
    std::chrono::time_point<std::chrono::system_clock> t0 = std::chrono::system_clock::now();
    Long64_t result = m_filterEventTree->Process(&stoelist, "", step0, 0);

    if (result < 0) {
      fwLog(fwlog::kWarning) << "FWFileEntry::runFilter in file [" << m_file->GetName() << "] filter ["
                            << filter->m_selector->m_expression << "] is invalid." << std::endl;
    }
    else {
      std::chrono::time_point<std::chrono::system_clock> t1 = std::chrono::system_clock::now();
      auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
      double ms = milliseconds.count();
      int stepsize = 1000 * step0 / ms;
      printf("%d events took %f => stepsize %d\n", step0, ms, stepsize);
      int offset = step0;
      while (offset < Ntotal)
      {
        if (navigator->getFilterState() == CmsShowNavigator::kOff)
            return;

        t0 = std::chrono::system_clock::now();
        result = m_filterEventTree->Process(&stoelist, "", stepsize, offset);
        t1 = std::chrono::system_clock::now();
        milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
        int msc = milliseconds.count();
        printf("offset %d result %d seconds %d\n", offset, filter->m_eventList->GetN(), msc);

        TString t = TString::Format("expr = %s offset = %d / %d selected = %d", filter->m_selector->m_expression.c_str(), offset, Ntotal, filter->m_eventList->GetN());

        ROOT::Experimental::REveManager::ChangeGuard ch;
        gui->SetTitle(t.Data());
        gui->StampObjProps();
        offset += stepsize;

        if (offset < 0) {
          fwLog(fwlog::kError) << "FWFileEntry::runFilter, invalid offset. Exiting \n";

          exit(0);
          return;
        }
      }


      fwLog(fwlog::kDebug) << "FWFileEntry::runFilter is file [" << m_file->GetName() << "], filter ["
                          << filter->m_selector->m_expression << "] has [" << filter->m_eventList->GetN()
                          << "] events selected" << std::endl;
    }
  }
  catch (std::exception &exc)
  {
    fwLog(fwlog::kWarning) << "FWFileEntry::runFilter in file [" << m_file->GetName() << "] filter ["
                           << filter->m_selector->m_expression << "] threw exception: " << exc.what() << std::endl;
  }

  filter->m_needsUpdate = false;
}

//______________________________________________________________________________

bool FWFileEntry::runHLTFilter(Filter* filterEntry, FWWebGUIEventFilter* gui) {
  std::string selection(filterEntry->m_selector->m_expression);

  boost::regex re_spaces("\\s+");
  selection = boost::regex_replace(selection, re_spaces, "");
  if (selection.find("&&") != std::string::npos && selection.find("||") != std::string::npos) {
    // Combination of && and || operators not supported.
    return false;
  }

  fwlite::Handle<edm::TriggerResults> hTriggerResults;
  edm::TriggerNames const* triggerNames(nullptr);
  try {
    hTriggerResults.getByLabel(*m_filterEvent, "TriggerResults", "", filterEntry->m_selector->m_triggerProcess.c_str());
    triggerNames = &(m_filterEvent->triggerNames(*hTriggerResults));
  } catch (...) {
    fwLog(fwlog::kWarning) << " failed to get trigger results with process name "
                           << filterEntry->m_selector->m_triggerProcess << std::endl;
    return false;
  }

  // std::cout << "Number of trigger names: " << triggerNames->size() << std::endl;
  // for (unsigned int i=0; i<triggerNames->size(); ++i)
  //  std::cout << " " << triggerNames->triggerName(i);
  // std::cout << std::endl;

  bool junction_mode = true;  // AND
  if (selection.find("||") != std::string::npos)
    junction_mode = false;  // OR

  boost::regex re("\\&\\&|\\|\\|");

  boost::sregex_token_iterator i(selection.begin(), selection.end(), re, -1);
  boost::sregex_token_iterator j;

  // filters and how they enter in the logical expression
  std::vector<std::pair<unsigned int, bool>> filters;

  while (i != j) {
    std::string filter = *i++;
    bool flag = true;
    if (filter[0] == '!') {
      flag = false;
      filter.erase(filter.begin());
    }
    unsigned int index = triggerNames->triggerIndex(filter);
    if (index == triggerNames->size()) {
      // Trigger name not found.
      return false;
    }
    filters.push_back(std::make_pair(index, flag));
  }
  if (filters.empty())
    return false;

  if (filterEntry->m_eventList)
    filterEntry->m_eventList->Reset();
  else
    filterEntry->m_eventList = new FW2TEventList();
  FW2TEventList* list = filterEntry->m_eventList;

  // loop over events
  edm::EventID currentEvent = m_filterEvent->id();
  unsigned int iEvent = 0;

  ROOT::Experimental::REveManager::ChangeGuard ch;
  gui->SetTitle(filterEntry->m_selector->m_triggerProcess.c_str());
  gui->StampObjProps();

  for (m_filterEvent->toBegin(); !m_filterEvent->atEnd(); ++(*m_filterEvent)) {
    hTriggerResults.getByLabel(*m_event, "TriggerResults", "", filterEntry->m_selector->m_triggerProcess.c_str());
    std::vector<std::pair<unsigned int, bool>>::const_iterator filter = filters.begin();
    bool passed = hTriggerResults->accept(filter->first) == filter->second;
    while (++filter != filters.end()) {
      if (junction_mode)
        passed &= hTriggerResults->accept(filter->first) == filter->second;
      else
        passed |= hTriggerResults->accept(filter->first) == filter->second;
    }
    if (passed)
      list->Enter(iEvent);
    ++iEvent;
  }
  m_filterEvent->to(currentEvent);

  filterEntry->m_needsUpdate = false;

  fwLog(fwlog::kDebug) << "FWFile::filterEventsWithCustomParser file [" << m_file->GetName() << "], filter ["
                       << filterEntry->m_selector->m_expression << "], selected [" << list->GetN() << "]" << std::endl;

  return true;
}

//------------------------------------------------------------------------------

FWTTreeCache* FWFileEntry::fwTreeCache() {
  FWTTreeCache* tc = dynamic_cast<FWTTreeCache*>(m_file->GetCacheRead(m_eventTree));
  assert(tc != nullptr && "FWFileEntry::treeCache can not access TTreeCache");
  return tc;
}

std::string FWFileEntry::getBranchName(const FWWebEventItem* it) const {
  const edm::TypeWithDict elementType(const_cast<TClass*>(it->type()));
  const edm::TypeWithDict wrapperType = edm::TypeWithDict::byName(edm::wrappedClassName(elementType.name()));
  return m_event->getBranchNameFor(
      wrapperType.typeInfo(), it->moduleLabel().c_str(), it->productInstanceLabel().c_str(), it->processName().c_str());
}

void FWFileEntry::NewEventItemCallIn(const FWWebEventItem* it) {
  auto tc = fwTreeCache();

  if (FWTTreeCache::IsLogging())
    printf("FWFileEntry:NewEventItemCallIn FWWebEventItem %s, learning=%d\n", getBranchName(it).c_str(), tc->IsLearning());

  tc->AddBranchTopLevel(getBranchName(it).c_str());
}

void FWFileEntry::RemovingEventItemCallIn(const FWWebEventItem* it) {
  auto tc = fwTreeCache();

  if (FWTTreeCache::IsLogging())
    printf("FWFileEntry:RemovingEventItemCallIn FWWebEventItem %s, learning=%d\n",
           getBranchName(it).c_str(),
           tc->IsLearning());

  tc->DropBranchTopLevel(getBranchName(it).c_str());
}
