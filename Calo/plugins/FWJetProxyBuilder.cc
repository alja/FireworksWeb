#ifndef Fireworks2_Calo_FW2JetProxyBuilder_h
#define Fireworks2_Calo_FW2JetProxyBuilder_h

#include "ROOT/REveDataSimpleProxyBuilderTemplate.hxx"
#include "ROOT/REveJetCone.hxx"
#include "ROOT/REveScalableStraightLineSet.hxx"
#include "ROOT/REveViewContext.hxx"

#include "Fireworks2/Core/interface/Context.h"
#include "Fireworks2/Calo/interface/makeEveJetCone.h"

#include "DataFormats/JetReco/interface/CaloJet.h"
#include "DataFormats/JetReco/interface/CaloJetCollection.h"

using namespace ROOT::Experimental;

class FWJetProxyBuilder: public REveDataSimpleProxyBuilderTemplate<reco::CaloJet>
{
   virtual bool HaveSingleProduct() const { return false; }
   
   using REveDataSimpleProxyBuilderTemplate<reco::CaloJet>::BuildViewType;
   virtual void BuildViewType(const reco::CaloJet& dj, REX::REveElement* iItemHolder, std::string viewType, const REX::REveViewContext*)
   {
      fireworks::Context* context = fireworks::Context::getInstance();
      
      REveJetCone* cone = fireworks::makeEveJetCone(dj, context);
      SetupAddElement(cone, iItemHolder, true);
 
      REveVector p1;
      REveVector p2;

      float size = 1.f; // values are saved in scale
      double theta = dj.theta();
      double phi = dj.phi();

      if (viewType == "Projected" ) // === RhoZ 
      {

         static const float_t offr = 4;
         float r_ecal = context->caloR1() + offr;
         float z_ecal = context->caloZ1() + offr/tan(context->caloTransAngle());
         double r(0);
         if ( theta < context->caloTransAngle() || M_PI-theta < context->caloTransAngle())
         {
            z_ecal = context->caloZ2() + offr/tan(context->caloTransAngle());
            r = z_ecal/fabs(cos(theta));
         }
         else
         {
            r = r_ecal/sin(theta);
         }

         p1.Set( 0., (phi>0 ? r*fabs(sin(theta)) : -r*fabs(sin(theta))), r*cos(theta));
         p2.Set( 0., (phi>0 ? (r+size)*fabs(sin(theta)) : -(r+size)*fabs(sin(theta))), (r+size)*cos(theta) );



         auto marker = new REX::REveScalableStraightLineSet("jetline");
         marker->SetScaleCenter(p1.fX, p1.fY, p1.fZ);
         marker->AddLine(p1, p2);
         std::cerr << "========================JET marlers \n";
         p1.Dump();
         p2.Dump();
         
         marker->SetScale(dj.et() * 50); // TODO :: implement scales
         marker->SetLineWidth(4);
         
         SetupAddElement(marker, iItemHolder, true);
      }
     
   }
};

#endif
