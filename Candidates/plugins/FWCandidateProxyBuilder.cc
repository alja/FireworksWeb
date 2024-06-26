// -*- C++ -*-
//
// Package:     Candidates
// Class  :     FWCandidateProxyBuilder
//
// Implementation:
//     <Notes on implementation>
//
// Original Author:  Chris Jones
//         Created:  Fri Dec  5 09:56:09 EST 2008
//


#include "ROOT/REveTrack.hxx"
#include "ROOT/REveTrackPropagator.hxx"
#include "ROOT/REveViewContext.hxx"
#include "ROOT/REveDataSimpleProxyBuilderTemplate.hxx"

#include "DataFormats/Candidate/interface/Candidate.h"

#include "FireworksWeb/Core/interface/FWProxyBuilderFactory.h"
#include "FireworksWeb/Candidates/interface/CandidateUtils.h"
#include "FireworksWeb/Core/interface/FWWebEventItem.h"
#include "FireworksWeb/Core/interface/FWProxyBuilderConfiguration.h"
#include "FireworksWeb/Core/interface/Context.h"
#include "FireworksWeb/Tracks/interface/estimate_field.h"
#include "FireworksWeb/Core/interface/Context.h"
#include "FireworksWeb/Core/interface/FWMagField.h"

using namespace ROOT::Experimental;

class FWCandidateProxyBuilder : public REveDataSimpleProxyBuilderTemplate<reco::Candidate>
{
public:
   REGISTER_FWPB_METHODS();

   using REveDataSimpleProxyBuilderTemplate<reco::Candidate>::BuildItem;
   virtual void BuildItem(const reco::Candidate& iData, int /*idx*/, REveElement* iItemHolder, const REveViewContext* vc) override
   {    
      auto context = fireworks::Context::getInstance();
      REveTrack* trk = fireworks::prepareCandidate(iData, context->getTrackPropagator());
      trk->MakeTrack();
      SetupAddElement(trk, iItemHolder);

      // if (item()->getConfig()->value<bool>("Draw backward extrapolation")) {
      if (false)
      {
         REveRecTrack t;
         t.fBeta = 1.;
         t.fV = REveVector(iData.vx(), iData.vy(), iData.vz());
         t.fP = REveVector(-iData.p4().px(), -iData.p4().py(), -iData.p4().pz());
         t.fSign = iData.charge();
         REveTrack* trk2 = new REveTrack(&t, context->getTrackPropagator());
         trk2->SetLineStyle(7);
         trk2->MakeTrack();
         SetupAddElement(trk2, iItemHolder);
      }
   }
};


REGISTER_FW2PROXYBUILDER(FWCandidateProxyBuilder, reco::Candidate, "Candidates");
