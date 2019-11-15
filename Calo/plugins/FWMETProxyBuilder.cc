// -*- C++ -*-
//
// Package:     Calo
// Class  :     FWMETProxyBuilder
//
// Implementation:
//     <Notes on implementation>
//
// Original Author:
//         Created:  Sun Jan  6 23:57:00 EST 2008
//

// system include files
#include "ROOT/REveScalableStraightLineSet.hxx"
#include "ROOT/REveGeoShape.hxx"
#include "TGeoTube.h"
#include "TMath.h"

#include "Fireworks2/Core/interface/FWProxyBuilderFactory.h"
#include "Fireworks2/Core/interface/BuilderUtils.h"
#include "Fireworks2/Core/interface/Context.h"
#include "Fireworks2/Calo/interface/scaleMarker.h"

#include "ROOT/REveDataSimpleProxyBuilderTemplate.hxx"
#include "ROOT/REveViewContext.hxx"
#include "DataFormats/METReco/interface/MET.h"

using namespace ROOT::Experimental;

class FWMETProxyBuilder: public REveDataSimpleProxyBuilderTemplate<reco::MET>
{
public:
   REGISTER_FWPB_METHODS();
   virtual bool HaveSingleProduct() const { return false; }

   using REveDataSimpleProxyBuilderTemplate<reco::MET>::BuildViewType;
  virtual void BuildViewType(const reco::MET& met, int /*idx*/, ROOT::Experimental::REveElement* iItemHolder, std::string viewType, const REveViewContext*) override
   {
      using namespace  TMath;
      double phi  = met.phi();
      double theta = met.theta();
      double size = 1.f;

      REveScalableStraightLineSet* marker = new REveScalableStraightLineSet( "MET marker" );
      marker->SetLineWidth( 2 );

      fireworks::Context* context = fireworks::Context::getInstance();

      if (viewType == "RhoZ" ) // === RhoZ
      {
         // body
         double r0;
         if (TMath::Abs(met.eta()) < context->caloTransEta())
         {
            r0  = context->caloR1()/sin(theta);
         }
         else
         {
            r0  = context->caloZ1()/fabs(cos(theta));
         }
         marker->SetScaleCenter( 0., Sign(r0*sin(theta), phi), r0*cos(theta) );
         double r1 = r0 + 1;
         marker->AddLine( 0., Sign(r0*sin(theta), phi), r0*cos(theta),
                          0., Sign(r1*sin(theta), phi), r1*cos(theta) );

         // arrow pointer
         double r2 = r1 - 0.1;
         double dy = 0.05*size;
         marker->AddLine( 0., Sign(r2*sin(theta) + dy*cos(theta), phi), r2*cos(theta) -dy*sin(theta),
                          0., Sign(r1*sin(theta), phi), r1*cos(theta) );
         dy = -dy;
         marker->AddLine( 0., Sign(r2*sin(theta) + dy*cos(theta), phi), r2*cos(theta) -dy*sin(theta),
                          0., Sign(r1*sin(theta), phi), r1*cos(theta) );

         // segment
         fireworks::addRhoZEnergyProjection( this, iItemHolder, context->caloR1() -1, context->caloZ1() -1,
                                             theta - 0.04, theta + 0.04,
                                             phi );
      }
      else
      {
         // body
         double r0 = context->caloR1();
         double r1 = r0 + 1;
         marker->SetScaleCenter( r0*cos(phi), r0*sin(phi), 0 );
         marker->AddLine( r0*cos(phi), r0*sin(phi), 0,
                          r1*cos(phi), r1*sin(phi), 0);

         // arrow pointer, xy  rotate offset point ..
         double r2 = r1 - 0.1;
         double dy = 0.05*size;

         marker->AddLine( r2*cos(phi) -dy*sin(phi), r2*sin(phi) + dy*cos(phi), 0,
                          r1*cos(phi), r1*sin(phi), 0);
         dy = -dy;
         marker->AddLine( r2*cos(phi) -dy*sin(phi), r2*sin(phi) + dy*cos(phi), 0,
                          r1*cos(phi), r1*sin(phi), 0);

         // segment
         double min_phi = phi-M_PI/36/2;
         double max_phi = phi+M_PI/36/2;
         REveGeoManagerHolder gmgr(REveGeoShape::GetGeoManager());
         REveGeoShape *element = fireworks::getShape( "spread", new TGeoTubeSeg( r0 - 2, r0, 1, min_phi*180/M_PI, max_phi*180/M_PI ), 0 );
         element->SetPickable( kTRUE );
         SetupAddElement( element, iItemHolder );
      }

      marker->SetScale(context->energyScale() * met.et());
      SetupAddElement( marker, iItemHolder );

      // printf("add line %s  %f %f .... eta %f theta %f\n", item()->name().c_str(), met.et(), met.energy(), met.eta(), met.theta());
      // m_lines.push_back(fireworks::scaleMarker(marker, met.et(), met.energy(), vc));  // register for scales
   }
};



REGISTER_FW2PROXYBUILDER(FWMETProxyBuilder, reco::MET, "recoMET");
