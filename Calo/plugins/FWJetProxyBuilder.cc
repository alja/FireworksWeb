#ifndef Fireworks2_Calo_FW2JetProxyBuilder_h
#define Fireworks2_Calo_FW2JetProxyBuilder_h

#include "ROOT/REveDataSimpleProxyBuilderTemplate.hxx"
#include "ROOT/REveJetCone.hxx"
#include "ROOT/REveScalableStraightLineSet.hxx"
#include "ROOT/REveViewContext.hxx"

#include "Fireworks2/Core/interface/Context.h"
#include "Fireworks2/Calo/interface/makeEveJetCone.h"
#include "Fireworks2/Core/interface/FWProxyBuilderFactory.h"

#include "DataFormats/JetReco/interface/Jet.h"

using namespace ROOT::Experimental;

class FWJetProxyBuilder: public REveDataSimpleProxyBuilderTemplate<reco::Jet>
{
public:
   REGISTER_FWPB_METHODS();
   virtual bool HaveSingleProduct() const { return false; }

   using REveDataSimpleProxyBuilderTemplate<reco::Jet>::BuildViewType;
   virtual void BuildViewType(const reco::Jet& dj, REveElement* iItemHolder, std::string viewType, const REveViewContext*)
   {
      fireworks::Context* context = fireworks::Context::getInstance();

      REveJetCone* cone = fireworks::makeEveJetCone(dj, context);
      SetupAddElement(cone, iItemHolder, true);

      if (viewType.compare(0, 3,  "Rho")) {
         REveVector p1;
         REveVector p2;

         float size = 1.f; // values are saved in scale
         double theta = dj.theta();
         double phi = dj.phi();

         if (viewType == "RhoZ" )
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


         }
         else if (viewType == "RhoPhi") {
            float ecalR = context->caloR1() + 4;
            p1.Set(ecalR*cos(phi), ecalR*sin(phi), 0);
            p2.Set((ecalR+size)*cos(phi), (ecalR+size)*sin(phi), 0);

         }

         auto marker = new ROOT::Experimental::REveScalableStraightLineSet("jetline");
         marker->SetScaleCenter(p1.fX, p1.fY, p1.fZ);
         marker->AddLine(p1, p2);

         marker->SetScale(dj.et() * context->energyScale()); // TODO :: implement scales
         marker->SetLineWidth(4);

         SetupAddElement(marker, iItemHolder, true);
      }
   }
};

REGISTER_FW2PROXYBUILDER(FWJetProxyBuilder, reco::Jet, "Jets");
#endif
