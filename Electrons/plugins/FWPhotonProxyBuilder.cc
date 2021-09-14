#ifndef FireworksWeb_Photons_FW2PhotonProxyBuilder_h
#define FireworksWeb_Photons_FW2PhotonProxyBuilder_h


// -*- C++ -*-
//
// Package:     Photons
// Class  :     FWPhotonProxyBuilder
//
// Implementation:
//     <Notes on implementation>
//
// Original Author:  Chris Jones
//         Created:  Tue Dec  2 14:17:03 EST 2008
//
#include "ROOT/REveTrack.hxx"
#include "ROOT/REveGeoShape.hxx"
#include "ROOT/REveDataCollection.hxx"
#include "ROOT/REveScalableStraightLineSet.hxx"
#include "ROOT/REveDataSimpleProxyBuilderTemplate.hxx"

#include "FireworksWeb/Core/interface/Context.h"
#include "FireworksWeb/Candidates/interface/CandidateUtils.h"
#include "FireworksWeb/Tracks/interface/TrackUtils.h"
#include "FireworksWeb/Electrons/interface/makeSuperCluster.h"
#include "FireworksWeb/Core/interface/FWProxyBuilderFactory.h"

#include "DataFormats/EgammaCandidates/interface/Photon.h"

////////////////////////////////////////////////////////////////////////////////
//
//   3D and RPZ proxy builder with shared track list
//
////////////////////////////////////////////////////////////////////////////////
using namespace ROOT::Experimental;

class FWPhotonProxyBuilder : public REveDataSimpleProxyBuilderTemplate<reco::Photon>
{
public:
   REGISTER_FWPB_METHODS();
   virtual bool HaveSingleProduct() const {return false;  }


   using REveDataSimpleProxyBuilderTemplate<reco::Photon>::BuildViewType;
   virtual void BuildViewType(const reco::Photon& photon, int /*idx*/,  REveElement* oItemHolder, const std::string& type , const REveViewContext*) override
   {
      
  if (type == "RPhi") {
    fireworks::makeRhoPhiSuperCluster(this, photon.superCluster(), photon.phi(), oItemHolder);
  }
  else if (type == "RhoZ")
    fireworks::makeRhoZSuperCluster(this, photon.superCluster(), photon.phi(), oItemHolder);

      
   }
};

REGISTER_FW2PROXYBUILDER(FWPhotonProxyBuilder, reco::Photon, "Photons");
#endif

