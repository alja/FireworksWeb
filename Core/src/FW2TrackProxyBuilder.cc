#ifndef Fireworks2_Core_FW2TrackProxyBuilder_h
#define Fireworks2_Core_FW2TrackProxyBuilder_h

#include "ROOT/REveTrack.hxx"
#include "ROOT/REveTrackPropagator.hxx"
#include "ROOT/REveViewContext.hxx"
#include "ROOT/REveDataSimpleProxyBuilderTemplate.hxx"
#include "TParticle.h"
#include "DataFormats/TrackReco/interface/Track.h"

namespace REX = ROOT::Experimental;

class FW2TrackProxyBuilder : public REX::REveDataSimpleProxyBuilderTemplate<reco::Track>
{   
   using REveDataSimpleProxyBuilderTemplate<reco::Track>::Build;
   virtual void Build(const reco::Track& p, REX::REveElement* iItemHolder, const REX::REveViewContext* context)
   {
       // printf("==============  BUILD track %s (pt=%f, eta=%f) \n", iItemHolder->GetCName(), p.pt(), p.eta());
      //TParticle tpart(pdg, Int_t status, Int_t mother1, Int_t mother2, Int_t daughter1, Int_t daughter2, const TLorentzVector &p, const TLorentzVector &v)
      // !!! NEED REveRecTrack ... this is fake !!!!

      // 	TParticle (Int_t pdg, Int_t status, Int_t mother1, Int_t mother2, Int_t daughter1, Int_t daughter2, Double_t px, Double_t py, Double_t pz, Double_t etot, Double_t vx, Double_t vy, Double_t vz, Double_t time)
      TParticle tpart(11, 0, 1, 1,1,1, (double)p.px(), (double)p.py(), (double)p.pz(), 1., p.vx(), p.vy(), p.vz(), 1.);
      auto track = new REX::REveTrack(&tpart, 1, context->GetPropagator());
      
      track->MakeTrack();
      SetupAddElement(track, iItemHolder, true);
      // iItemHolder->AddElement(track);
      // track->Print("all");
      track->SetName(Form("element %s id=%d", iItemHolder->GetCName(), track->GetElementId()));
   }
};

#endif
