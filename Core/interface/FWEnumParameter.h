#ifndef FireworksWeb_Core_FWEnumParameter_h
#define FireworksWeb_Core_FWEnumParameter_h
// -*- C++ -*-
//
// Package:     Core
// Class  :     FWEnumParameter
//
/**\class FWEnumParameter FWEnumParameter.h FireworksWeb/Core/interface/FWEnumParameter.h

 Description: Specialization of FWLongParameter to allow drop-down menu GUI.

 Usage:
    <usage>

*/
//
// Original Author:  matevz
//         Created:  Fri Apr 30 15:16:55 CEST 2010
//

// system include files

// user include files
#include "FireworksWeb/Core/interface/FWLongParameter.h"
#include "RtypesCore.h"
#include <map>

// forward declarations

class FWEnumParameter : public FWLongParameter {
public:
  FWEnumParameter() : FWLongParameter() {}

  FWEnumParameter(
      FWParameterizable* iParent, const std::string& iName, const long& iDefault = 0, long iMin = -1, long iMax = -1)
      : FWLongParameter(iParent, iName, iDefault, iMin, iMax) {}

  template <class K>
  FWEnumParameter(FWParameterizable* iParent,
                  const std::string& iName,
                  K iCallback,
                  const long& iDefault = 0,
                  long iMin = -1,
                  long iMax = -1)
      : FWLongParameter(iParent, iName, iCallback, iDefault, iMin, iMax) {}

  // ---------- const member functions ---------------------

  // ---------- static member functions --------------------

  // ---------- member functions ---------------------------

  bool addEntry(Long_t id, const std::string& txt) { return m_enumEntries.insert(std::make_pair(id, txt)).second; }

  const std::map<Long_t, std::string>& entryMap() const { return m_enumEntries; }

  FWEnumParameter(const FWEnumParameter&) = delete;                   // stop default
  const FWEnumParameter& operator=(const FWEnumParameter&) = delete;  // stop default

private:
  // ---------- member data --------------------------------
  std::map<Long_t, std::string> m_enumEntries;
};

#endif
