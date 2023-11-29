#ifndef FireworksWeb_Core_FWRPZViewGeometry_h
#define FireworksWeb_Core_FWRPZViewGeometry_h
// -*- C++ -*-
//
// Package:     Core
// Class  :     FWRPZViewGeometry
//
/**\class FWRPZViewGeometry FWRPZViewGeometry.h FireworksWeb/Core/interface/FWRPZViewGeometry.h

 Description: [one line class summary]

 Usage:
    <usage>

*/
//
// Original Author:  Alja Mrak-Tadel
//         Created:  Thu Mar 25 21:01:12 CET 2010
//

// system include files

// user include files
#include "ROOT/REveElement.hxx"
#include "FireworksWeb/Core/interface/FWGeometry.h"

// forward declarations
class TGeoShape;

namespace fireworks {
  class Context;
}

class FWRPZViewGeometry : public ROOT::Experimental::REveElement {
public:
  FWRPZViewGeometry();
  ~FWRPZViewGeometry() override;

  // ---------- const member functions ---------------------

  // ---------- static member functions --------------------

  // ---------- member functions ---------------------------
  void initStdGeoElements(const std::string& viewType);
/*
  void showPixelBarrel(bool);
  void showPixelEndcap(bool);
  void showTrackerBarrel(bool);
  void showTrackerEndcap(bool);
  void showRpcEndcap(bool);
  void showGEM(bool);
  void showME0(bool);
  */
  ROOT::Experimental::REveElement* makeMuonGeometryRhoPhi();
  ROOT::Experimental::REveElement* makeMuonGeometryRhoZ();
  ROOT::Experimental::REveElement* makeCaloOutlineRhoPhi();
  ROOT::Experimental::REveElement* makeCaloOutlineRhoZ();

private:
  FWRPZViewGeometry(const FWRPZViewGeometry&);                   // stop default
  const FWRPZViewGeometry& operator=(const FWRPZViewGeometry&);  // stop default

  void addToCompound(ROOT::Experimental::REveElement*, Color_t, bool transp = false);
  // ---------- member data --------------------------------

  const fireworks::Context* m_context{nullptr};
  const FWGeometry* m_geom{nullptr};

  void estimateProjectionSizeDT(const FWGeometry::GeomDetInfo& info, float&, float&, float&, float&);
  void estimateProjectionSizeCSC(const FWGeometry::GeomDetInfo& info, float&, float&, float&, float&);
  void estimateProjectionSize(const float*, float&, float&, float&, float&);

  // void importNew(ROOT::Experimental::REveElement* x);

  ROOT::Experimental::REveGeoShape* makeShape(double, double, double, double);

  ROOT::Experimental::REveElement* m_rhoPhiGeo;
  ROOT::Experimental::REveElement* m_rhoZGeo;

  ROOT::Experimental::REveElement* m_pixelBarrelElements;
  ROOT::Experimental::REveElement* m_pixelEndcapElements;
  ROOT::Experimental::REveElement* m_trackerBarrelElements;
  ROOT::Experimental::REveElement* m_trackerEndcapElements;
  ROOT::Experimental::REveElement* m_rpcEndcapElements;
  ROOT::Experimental::REveElement* m_GEMElements;
  ROOT::Experimental::REveElement* m_ME0Elements;
};

#endif
