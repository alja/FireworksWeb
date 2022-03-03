#ifndef Fireworks_Core_FWWebInvMassDialog_h
#define Fireworks_Core_FWWebInvMassDialog_h
// -*- C++ -*-
//
// Package:     Core
// Class  :     FWWebInvMassDialog
//
/**\class FWWebInvMassDialog FWWebInvMassDialog.h FireworksWeb/Core/interface/FWWebInvMassDialog.h

 Description: [one line class summary]

 Usage:
    <usage>

*/
//
// Original Author:  Matevz Tadel
//         Created:  Mon Nov 22 11:05:41 CET 2010
//

#include "ROOT/REveElement.hxx"
// forward declarations

class FWWebInvMassDialog : public ROOT::Experimental::REveElement {
public:
  FWWebInvMassDialog();
  ~FWWebInvMassDialog() override;

  void Calculate();

protected:
  void addLine(const TString& line);
private:
  FWWebInvMassDialog(const FWWebInvMassDialog&);  // stop default

  const FWWebInvMassDialog& operator=(const FWWebInvMassDialog&);  // stop default
  int WriteCoreJson(nlohmann::json &j, int rnr_offset) override;

};

#endif
