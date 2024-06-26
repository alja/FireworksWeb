#ifndef FireworksWeb_Core_FWViewGeometryList_h
#define FireworksWeb_Core_FWViewGeometryList_h
// -*- C++ -*-
//
// Package:     Core
// Class  :     FWViewGeometryList
//
/**\class FWViewGeometryList FWViewGeometryList.h Fireworks/Core/interface/FWViewGeometryList.h

 Description: [one line class summary]

 Usage:
    <usage>

*/
//
// Original Author:  Alja Mrak Tadel
//         Created:  Tue Sep 14 13:28:39 CEST 2010
//

#include "sigc++/connection.h"

#include "ROOT/REveElement.hxx"
// #include "ROOT/REveProjectionBases.hxx"
#include "ROOT/REveCompound.hxx"
#include "FireworksWeb/Core/interface/FWColorManager.h"

class TGeoMatrix;
class FWGeometry;


namespace fireworks {
  class Context;
}

class ColorAuntie : public ROOT::Experimental::REveElement, public ROOT::Experimental::REveAuntAsList
{
  public:
  ColorAuntie(const std::string &name = "", const std::string &title = "") : REveElement(name, title) {};
};

class FWViewGeometryList : public ROOT::Experimental::REveCompound {
public:
  FWViewGeometryList(const fireworks::Context& context, bool projected = true);
  ~FWViewGeometryList() override;

  void updateColors();
  void updateTransparency(bool projectedType);

protected:
  const fireworks::Context& m_context;
  const FWGeometry* m_geom;  // cached

  ColorAuntie* m_colorComp[kFWGeomColorSize];

  void addToAunt(ROOT::Experimental::REveElement* el, FWGeomColorIndex idx, bool applyTransp = true) const;

public:
  FWViewGeometryList(const FWViewGeometryList&) = delete;  // stop default

  const FWViewGeometryList& operator=(const FWViewGeometryList&) = delete;  // stop default

private:
  // ---------- member data --------------------------------
  sigc::connection m_transpConnection;
  sigc::connection m_colorConnection;
  bool m_projected;  // needed for transparency
};

#endif
