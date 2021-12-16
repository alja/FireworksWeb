/*
 *  FWBeamSpotProxyBuilder.cc
 *  FWorks
 *
 *  Created by Ianna Osborne on 7/29/10.
 *
 */
#include "ROOT/REveDataSimpleProxyBuilderTemplate.hxx"
#include "ROOT/REveStraightLineSet.hxx"

#include "DataFormats/BeamSpot/interface/BeamSpot.h"

#include "FireworksWeb/Core/interface/FWEventItem.h"
#include "FireworksWeb/Core/interface/FWProxyBuilderFactory.h"

using namespace ROOT::Experimental;

class FWBeamSpotProxyBuilder : public REveDataSimpleProxyBuilderTemplate<reco::BeamSpot>
{
public:
   REGISTER_FWPB_METHODS();

private:
   using REveDataSimpleProxyBuilderTemplate<reco::BeamSpot>::BuildItem;
   virtual void BuildItem(const reco::BeamSpot& bs, int /*idx*/, REveElement* oItemHolder, const REveViewContext* vc) override
   {
      REveStraightLineSet* ls = new REveStraightLineSet();

      double pos[3] = {bs.x0(), bs.y0(), bs.z0()};
      double e[3] = {bs.x0Error(), bs.y0Error(), bs.z0Error()};

      const Int_t N = 32;
      const Float_t S = 2 * TMath::Pi() / N;

      Float_t a = e[0], b = e[1];
      for (Int_t i = 0; i < N; i++)
         ls->AddLine(
                     a * TMath::Cos(i * S), b * TMath::Sin(i * S), 0, a * TMath::Cos(i * S + S), b * TMath::Sin(i * S + S), 0);

      a = e[0];
      b = e[2];
      for (Int_t i = 0; i < N; i++)
         ls->AddLine(
                     a * TMath::Cos(i * S), 0, b * TMath::Sin(i * S), a * TMath::Cos(i * S + S), 0, b * TMath::Sin(i * S + S));

      a = e[1];
      b = e[2];
      for (Int_t i = 0; i < N; i++)
         ls->AddLine(
                     0, a * TMath::Cos(i * S), b * TMath::Sin(i * S), 0, a * TMath::Cos(i * S + S), b * TMath::Sin(i * S + S));

      ls->AddLine(0, 0, 0, 0, 0, 0);
      ls->AddMarker(0, 0, 0);
      ls->SetMarkerStyle(21);
      //  const FWDisplayProperties& dp = FWProxyBuilderBase::item()->defaultDisplayProperties();
      // ls->SetMarkerColor(dp.color());

      ls->RefMainTrans().SetPos(pos);
      SetupAddElement(ls, oItemHolder, true);
   }
};

REGISTER_FW2PROXYBUILDER(FWBeamSpotProxyBuilder, reco::BeamSpot, "Beam Spot");

