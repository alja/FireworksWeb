#ifndef FireworksWeb_Core_FW2TEventList_h
#define FireworksWeb_Core_FW2TEventList_h

// There was a bug in ROOT ... fixed on Dec 9 2009:
//   http://root.cern.ch/viewcvs/trunk/tree/tree/src/TEventList.cxx?view=log
//
// We need to keep this intermediate class until we switch to
// root-5.26 or later.

#include "TEventList.h"

class FW2TEventList : public TEventList {
public:
  FW2TEventList() : TEventList() {}
  FW2TEventList(const char* name, const char* title = "", Int_t initsize = 0, Int_t delta = 0)
      : TEventList(name, title, initsize, delta) {}

  ~FW2TEventList() override {}

  void Enter(Long64_t entry) override;
  void Add(const TEventList* list) override;

private:
  FW2TEventList(const FW2TEventList&);                   // stop default
  const FW2TEventList& operator=(const FW2TEventList&);  // stop default

  ClassDefOverride(FW2TEventList, 0);
};

#endif
