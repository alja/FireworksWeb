// -*- C++ -*-
//
// Package:     Core
// Class  :     FWConfigurationManager
//
// Implementation:
//     <Notes on implementation>
//
// Original Author:  Chris Jones
//         Created:  Sun Feb 24 14:42:32 EST 2008
//

// system include files
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include "curl/curl.h"
#include "TROOT.h"
#include "TSystem.h"
#include "TStopwatch.h"

// user include files
#include "FireworksWeb/Core/interface/FWConfigurationManager.h"
#include "FireworksWeb/Core/interface/FWConfiguration.h"
#include "FireworksWeb/Core/interface/FWConfigurable.h"
#include "FireworksWeb/Core/interface/fwLog.h"
#include "FireworksWeb/Core/interface/SimpleSAXParser.h"
#include "FireworksWeb/Core/interface/FWJobMetadataManager.h"
#include "FireworksWeb/Core/interface/FWXMLConfigParser.h"

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
FWConfigurationManager::FWConfigurationManager() : m_ignore(false) {}

// FWConfigurationManager::FWConfigurationManager(const FWConfigurationManager& rhs)
// {
//    // do actual copying here;
// }

FWConfigurationManager::~FWConfigurationManager() {}

//
// assignment operators
//
// const FWConfigurationManager& FWConfigurationManager::operator=(const FWConfigurationManager& rhs)
// {
//   //An exception safe implementation is
//   FWConfigurationManager temp(rhs);
//   swap(rhs);
//
//   return *this;
// }

//
// member functions
//
void FWConfigurationManager::add(const std::string& iName, FWConfigurable* iConf) {
  assert(nullptr != iConf);
  m_configurables[iName] = iConf;
}

//
// const member functions
//
void FWConfigurationManager::setFrom(const FWConfiguration& iConfig) const {
  assert(nullptr != iConfig.keyValues());
  for (FWConfiguration::KeyValues::const_iterator it = iConfig.keyValues()->begin(), itEnd = iConfig.keyValues()->end();
       it != itEnd;
       ++it) {
    std::map<std::string, FWConfigurable*>::const_iterator itFound = m_configurables.find(it->first);
    assert(itFound != m_configurables.end());
    itFound->second->setFrom(it->second);
  }
}

void FWConfigurationManager::to(FWConfiguration& oConfig) const {
  FWConfiguration config;
  for (std::map<std::string, FWConfigurable*>::const_iterator it = m_configurables.begin(),
                                                              itEnd = m_configurables.end();
       it != itEnd;
       ++it) {
    it->second->addTo(config);
    oConfig.addKeyValue(it->first, config, true);
  }
}

void FWConfigurationManager::writeToFile(const std::string& iName) const {
  try {
    std::ofstream file(iName.c_str());
    if (not file) {
      std::string message = "unable to open file " + iName;
      message += iName;
      throw std::runtime_error(message.c_str());
    }
    FWConfiguration top;
    to(top);
    fwLog(fwlog::kInfo) << "Writing to file " << iName.c_str() << "...\n";
    fflush(stdout);

    FWConfiguration::streamTo(file, top, "top");
  } catch (std::runtime_error& e) {
    fwLog(fwlog::kError) << "FWConfigurationManager::writeToFile() " << e.what() << std::endl;
  }
}


/** Reads the configuration specified in @a iName and creates the internal 
    representation in terms of FWConfigutation objects.
    
    Notice that if the file does not start with '<' the old CINT macro based
    system is used.
  */
void FWConfigurationManager::readFromFile(const std::string &iName) const
{
  if (iName.rfind("http", 0) == 0) 
    readFromRemoteFile(iName);
  else
    readFromLocalFile(iName);
}

void FWConfigurationManager::readFromLocalFile(const std::string &iName) const
{
  std::ifstream f(iName.c_str());

  // Check that the syntax is correct.
  SimpleSAXParser syntaxTest(f);
  syntaxTest.parse();
  f.close();

  // Read again, this time actually parse.
  std::ifstream g(iName.c_str());
  // Actually parse the results.
  FWXMLConfigParser parser(g);
  parser.parse();
  setFrom(*parser.config());
}

/*
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
std::cout << "write callback " << (char*)contents << "\n";
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}*/

void FWConfigurationManager::readFromRemoteFile(const std::string &url) const
{
  std::stringstream f;
  auto callback = +[](char *ptr_data, size_t size, size_t nmemb, std::string *writerData)
      -> size_t
  {
    if (writerData == NULL)
      return 0;

    if (size > 3000000)
    {
      std::cout << "Configuration file too large !!!\n";
      return 0;
    }
    size_t data_size = size * nmemb;
    writerData->append(ptr_data, data_size);
    return (int)data_size;
  };

  CURL *curl;
  std::string readBuffer;
  CURLcode res;
  curl = curl_easy_init();
  if (curl)
  {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    res =  curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res)
    {
      std::string es(curl_easy_strerror(res));
      throw (std::runtime_error(es) );
    }
    f << readBuffer;
  }

  // Check that the syntax is correct.
  SimpleSAXParser syntaxTest(f);
  syntaxTest.parse();

  // Read again, this time actually parse.
  std::stringstream g(readBuffer);
  // Actually parse the results.
  FWXMLConfigParser parser(g);
  parser.parse();
  setFrom(*parser.config());
}

std::string FWConfigurationManager::guessAndReadFromFile(FWJobMetadataManager* dataMng) const {
  struct CMatch {
    std::string file;
    int cnt;
    const FWConfiguration* cfg;

    CMatch(std::string f) : file(f), cnt(0), cfg(nullptr) {}
    bool operator<(const CMatch& x) const { return cnt < x.cnt; }
  };

  std::vector<CMatch> clist;
  clist.push_back(CMatch("reco.fwc"));
  clist.push_back(CMatch("miniaod.fwc"));
  clist.push_back(CMatch("aod.fwc"));
  std::vector<FWJobMetadataManager::Data>& sdata = dataMng->usableData();

  for (std::vector<CMatch>::iterator c = clist.begin(); c != clist.end(); ++c) {
    std::string iName = gSystem->Which(TROOT::GetMacroPath(), c->file.c_str(), kReadPermission);
    std::ifstream f(iName.c_str());
    if (f.peek() != (int)'<') {
      fwLog(fwlog::kWarning) << "FWConfigurationManager::guessAndReadFromFile can't open " << iName << std::endl;
      continue;
    }

    // Read again, this time actually parse.
    std::ifstream g(iName.c_str());
    FWXMLConfigParser* parser = new FWXMLConfigParser(g);
    parser->parse();

    c->cfg = parser->config();
    const FWConfiguration::KeyValues* keyValues = nullptr;
    for (FWConfiguration::KeyValues::const_iterator it = c->cfg->keyValues()->begin(),
                                                    itEnd = c->cfg->keyValues()->end();
         it != itEnd;
         ++it) {
      if (it->first == "EventItems") {
        keyValues = it->second.keyValues();
        break;
      }
    }

    for (FWConfiguration::KeyValues::const_iterator it = keyValues->begin(); it != keyValues->end(); ++it) {
      const FWConfiguration& conf = it->second;
      const FWConfiguration::KeyValues* keyValues = conf.keyValues();
      const std::string& type = (*keyValues)[0].second.value();
      for (std::vector<FWJobMetadataManager::Data>::iterator di = sdata.begin(); di != sdata.end(); ++di) {
        if (di->type_ == type) {
          c->cnt++;
          break;
        }
      }
    }
    // printf("%s file %d matches\n", iName.c_str(), c->cnt);
  }
  std::sort(clist.begin(), clist.end());
  fwLog(fwlog::kInfo) << "Loading configuration file " << clist.back().file << std::endl;
  setFrom(*(clist.back().cfg));

  return clist.back().file;
}

//
// static member functions
//
