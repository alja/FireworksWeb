#ifndef FireworksWeb_Core_FW3DViewGeometry_h
#define FireworksWeb_Core_FW3DViewGeometry_h
// -*- C++ -*-
//
// Package:     Core
// Class  :     FW3DViewGeometry
//
/**\class FW3DViewGeometry FW3DViewGeometry.h Fireworks/Core/interface/FW3DViewGeometry.h

 Description: [one line class summary]

 Usage:
    <usage>

*/
//
// Original Author:  Alja Mrak Tadel
//         Created:  Thu Mar 25 22:06:52 CET 2010
//

#include "FireworksWeb/Core/interface/FWViewGeometryList.h"

// forward declarations

namespace fireworks {
  class Context;
}

class FW3DViewGeometry : public FWViewGeometryList {
public:
  FW3DViewGeometry(const fireworks::Context& context);
  ~FW3DViewGeometry() override;

  // ---------- const member functions ---------------------

  // ---------- static member functions --------------------

  // ---------- member functions ---------------------------

  void showMuonBarrel(bool);
  void showMuonBarrelFull(bool);
  void showMuonEndcap(bool);
  void showPixelBarrel(bool);
  void showPixelEndcap(bool);
  void showTrackerBarrel(bool);
  void showTrackerEndcap(bool);
  void showHGCalEE(bool);
  ROOT::Experimental::REveCompound const* const getHGCalEE() { return m_HGCalEEElements; }
  void showHGCalHSi(bool);
  ROOT::Experimental::REveCompound const* const getHGCalHSi() { return m_HGCalHSiElements; }
  void showHGCalHSc(bool);
  ROOT::Experimental::REveCompound const* const getHGCalHSc() { return m_HGCalHScElements; }
  void showMtdBarrel(bool);
  void showMtdEndcap(bool);

  FW3DViewGeometry(const FW3DViewGeometry&) = delete;  // stop default

  const FW3DViewGeometry& operator=(const FW3DViewGeometry&) = delete;  // stop default

private:
  // ---------- member data --------------------------------

  ROOT::Experimental::REveCompound* m_muonBarrelElements;
  ROOT::Experimental::REveCompound* m_muonBarrelFullElements;
  ROOT::Experimental::REveCompound* m_muonEndcapElements;
  ROOT::Experimental::REveCompound* m_muonEndcapFullElements;
  ROOT::Experimental::REveCompound* m_pixelBarrelElements;
  ROOT::Experimental::REveCompound* m_pixelEndcapElements;
  ROOT::Experimental::REveCompound* m_trackerBarrelElements;
  ROOT::Experimental::REveCompound* m_trackerEndcapElements;
  ROOT::Experimental::REveCompound* m_HGCalEEElements;
  ROOT::Experimental::REveCompound* m_HGCalHSiElements;
  ROOT::Experimental::REveCompound* m_HGCalHScElements;
  ROOT::Experimental::REveCompound* m_mtdBarrelElements;
  ROOT::Experimental::REveCompound* m_mtdEndcapElements;
};

#endif
