#ifndef Fireworks2_Core_FW2JetProxyBuilder_h
#define Fireworks2_Core_FW2JetProxyBuilder_h

#include "ROOT/REveDataSimpleProxyBuilderTemplate.hxx"
#include "ROOT/REveJetCone.hxx"
#include "ROOT/REveScalableStraightLineSet.hxx"
#include "ROOT/REveViewContext.hxx"

#include "DataFormats/JetReco/interface/CaloJet.h"
#include "DataFormats/JetReco/interface/CaloJetCollection.h"

namespace REX = ROOT::Experimental;

class FW2JetProxyBuilder: public REX::REveDataSimpleProxyBuilderTemplate<reco::CaloJet>
{
   virtual bool HaveSingleProduct() const { return false; }
   
   using REveDataSimpleProxyBuilderTemplate<reco::CaloJet>::BuildViewType;
   virtual void BuildViewType(const reco::CaloJet& dj, REX::REveElement* iItemHolder, std::string viewType, const REX::REveViewContext* context)
   {
      auto jet = new REX::REveJetCone();
      jet->SetCylinder(context->GetMaxR(), context->GetMaxZ());
      float eta_size = 0.02;
      float phi_size = 0.02;
      jet->AddEllipticCone(dj.eta(), dj.phi(), eta_size, phi_size);
      SetupAddElement(jet, iItemHolder, true);

      REX::REveVector p1;
      REX::REveVector p2;

      float size = 50.f * dj.pt(); // values are saved in scale
      double theta = dj.theta();
      //  printf("%s jet theta =  %f, phi = %f \n",  iItemHolder->GetCName(), theta, dj.phi());
      double phi = dj.phi();


      if (viewType == "Projected" )
      {
         static const float_t offr = 6;
         float r_ecal = context->GetMaxR() + offr;
         float z_ecal = context->GetMaxZ() + offr;
         
         float transAngle = abs(atan(r_ecal/z_ecal));
         double r(0);
         bool debug = false;
         if ( theta < transAngle || 3.14-theta < transAngle)
         {
            z_ecal = context->GetMaxZ() + offr/transAngle;
            r = z_ecal/fabs(cos(theta));
         }
         else
         {
            debug = 3;
            r = r_ecal/sin(theta);
         }

         p1.Set( 0., (phi<TMath::Pi() ? r*fabs(sin(theta)) : -r*fabs(sin(theta))), r*cos(theta));
         p2.Set( 0., (phi<TMath::Pi() ? (r+size)*fabs(sin(theta)) : -(r+size)*fabs(sin(theta))), (r+size)*cos(theta) );


         auto marker = new REX::REveScalableStraightLineSet("jetline");
         marker->SetScaleCenter(p1.fX, p1.fY, p1.fZ);
         marker->AddLine(p1, p2);   
         
         marker->SetLineWidth(4);
         if (debug)
             marker->AddMarker(0, 0.9);
         
         SetupAddElement(marker, iItemHolder, true);
      }
     
      jet->SetName(Form("element %s", iItemHolder->GetName().c_str()));
   }
};

#endif
