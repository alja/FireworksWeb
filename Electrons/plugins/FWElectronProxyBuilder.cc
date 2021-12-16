#ifndef FireworksWeb_Electrons_FW2ElectronProxyBuilder_h
#define FireworksWeb_Electrons_FW2ElectronProxyBuilder_h


// -*- C++ -*-
//
// Package:     Electrons
// Class  :     FWElectronProxyBuilder
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

#include "DataFormats/EgammaCandidates/interface/GsfElectron.h"
#include "DataFormats/GsfTrackReco/interface/GsfTrack.h"

////////////////////////////////////////////////////////////////////////////////
//
//   3D and RPZ proxy builder with shared track list
//
////////////////////////////////////////////////////////////////////////////////
using namespace ROOT::Experimental;

class FWElectronProxyBuilder : public REveDataSimpleProxyBuilderTemplate<reco::GsfElectron>
{
public:
   REGISTER_FWPB_METHODS();
   virtual bool HaveSingleProduct() const {return false;  }


   using REveDataSimpleProxyBuilderTemplate<reco::GsfElectron>::BuildItemViewType;
   virtual void BuildItemViewType(const reco::GsfElectron& electron, int /*idx*/,  REveElement* oItemHolder, const std::string& type , const REveViewContext*) override
   {
      auto context = fireworks::Context::getInstance();
      REveTrack* track(nullptr);
      if( electron.gsfTrack().isAvailable() )
         track = fireworks::prepareTrack( *electron.gsfTrack(),
                                          context->getTrackPropagator());
      else
         track = fireworks::prepareCandidate( electron,
                                              context->getTrackPropagator());
      track->MakeTrack();
      track->SetLineWidth(2);
      SetupAddElement(track, oItemHolder );

      if( type == "RPhi" )
         fireworks::makeRhoPhiSuperCluster( this, electron.superCluster(), electron.phi(), oItemHolder );


      else if( type == "RhoZ" )
         fireworks::makeRhoZSuperCluster(this, electron.superCluster(), electron.phi(), oItemHolder );
   }
};

REGISTER_FW2PROXYBUILDER(FWElectronProxyBuilder, reco::GsfElectron, "Electrons");
#endif

