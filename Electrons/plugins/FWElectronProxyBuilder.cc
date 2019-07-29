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
#include "ROOT/REveDataClasses.hxx"
#include "ROOT/REveScalableStraightLineSet.hxx"
#include "ROOT/REveDataSimpleProxyBuilderTemplate.hxx"

#include "Fireworks2/Core/interface/Context.h"
#include "Fireworks2/Candidates/interface/CandidateUtils.h"
#include "Fireworks2/Tracks/interface/TrackUtils.h"
#include "Fireworks2/Electrons/interface/makeSuperCluster.h" 

#include "DataFormats/EgammaCandidates/interface/GsfElectron.h"
#include "DataFormats/GsfTrackReco/interface/GsfTrack.h"

////////////////////////////////////////////////////////////////////////////////
//
//   3D and RPZ proxy builder with shared track list
// 
////////////////////////////////////////////////////////////////////////////////
using namespace ROOT::Experimental;

class FWElectronProxyBuilder : public REveDataSimpleProxyBuilderTemplate<reco::GsfElectron> {

public:
   using REveDataProxyBuilderBase::HaveSingleProduct;
   bool  HaveSingleProduct() const override { return false; }
   
   using REveDataSimpleProxyBuilderTemplate<reco::GsfElectron>::BuildViewType;
  void BuildViewType(const reco::GsfElectron& iData, int /*idx*/, REveElement* oItemHolder, std::string type , const REveViewContext*) override;

private:
   void cleanLocal();
   
   // REveElement* requestCommon();
   // REveElement* m_common;
};

/*

REveElement*
FWElectronProxyBuilder::requestCommon()
{
   auto context = fireworks::Context::getInstance();
   if( m_common->HasChildren() == false )
   {
      int width = 2;
      for (int i = 0; i < static_cast<int>(Collection()->GetNItems()); ++i)
      {
         //         const reco::GsfElectron& electron = modelData(i);
         const reco::GsfElectron* electronPtr = static_cast<const reco::GsfElectron*>(Collection()->GetDataPtr(i));
         const reco::GsfElectron& electron = *electronPtr;
         REveTrack* track(nullptr);
         if( electron.gsfTrack().isAvailable() )
            track = fireworks::prepareTrack( *electron.gsfTrack(),
                                             context->getTrackPropagator());
         else
            track = fireworks::prepareCandidate( electron,
                                                 context->getTrackPropagator());
         track->MakeTrack();
         track->SetLineWidth(width);
         // setupElement( track );
         m_common->AddElement( track );
      }
   }
   return m_common;
   }*/

void
FWElectronProxyBuilder::cleanLocal()
{
   //m_common->DestroyElements();
}

void
FWElectronProxyBuilder::BuildViewType(const reco::GsfElectron& electron,  REveElement* oItemHolder, std::string type , const REveViewContext*)
{
   // AMT need new argument -- index in this function to share common ingredients
   
   /*
   REveElement*   tracks = requestCommon();
   REveElement::List_t trk = tracks->RefChildren();
   auto trkIt = trk.begin();
   std::advance(trkIt, iIndex);
   SetupAddElement(*trkIt, &oItemHolder );
   */

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
   std::cout << "ELEVTROJN =========================== ===\n";
   
   if( type == "RhoPhi" )
      fireworks::makeRhoPhiSuperCluster( this, electron.superCluster(), electron.phi(), oItemHolder );
                                         

   else if( type == "RhoZ" )
      fireworks::makeRhoZSuperCluster(this, electron.superCluster(), electron.phi(), oItemHolder );
}
