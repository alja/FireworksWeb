// -*- C++ -*-
//
// Package:     Electrons
// Class  :     makeSuperCluster
//
// Implementation:
//     <Notes on implementation>
//
// Original Author:  Chris Jones
//         Created:  Fri Dec  5 15:32:33 EST 2008
//

// system include files
#include "TGeoBBox.h"
#include "TGeoTube.h"

#include "ROOT/REveElement.hxx"
#include "ROOT/REveGeoShape.hxx"
#include "ROOT/REveUtil.hxx"
#include "ROOT/REveDataProxyBuilderBase.hxx"

// user include files
#include "FireworksWeb/Electrons/interface/makeSuperCluster.h"

#include "FireworksWeb/Core/interface/BuilderUtils.h"
#include "FireworksWeb/Core/interface/FWGeometry.h"
#include "FireworksWeb/Core/interface/Context.h"


using namespace ROOT::Experimental;

namespace fireworks {
bool makeRhoPhiSuperCluster( REveDataProxyBuilderBase* pb,
                             const reco::SuperClusterRef& iCluster,
                             float iPhi,
                             REveElement* oItemHolder )
{
   if( !iCluster.isAvailable()) return false;
   
   fireworks::Context* context = fireworks::Context::getInstance();
   ROOT::Experimental::REveGeoManagerHolder gmgr( REveGeoShape::GetGeoManager());
   
   std::vector< std::pair<DetId, float> > detids = iCluster->hitsAndFractions();
   if (detids.empty()) return false;
   std::vector<double> phis;
   for( std::vector<std::pair<DetId, float> >::const_iterator id = detids.begin(), end = detids.end(); id != end; ++id )
   {
      const float* corners = context->getGeom()->getCorners( id->first.rawId());
      if( corners != nullptr )
      {
         std::vector<float> centre( 3, 0 );

         for( unsigned int i = 0; i < 24; i += 3 )
         {	 
            centre[0] += corners[i];
            centre[1] += corners[i + 1];
            centre[2] += corners[i + 2];
         }
       
         phis.push_back( REveVector( centre[0], centre[1], centre[2] ).Phi());
      }
   }
   std::pair<double,double> phiRange = fireworks::getPhiRange( phis, iPhi );
   const double r = context->caloR1();
   TGeoBBox *sc_box = new TGeoTubeSeg( r - 2, r , 1,
                                       phiRange.first * 180 / M_PI - 0.5,
                                       phiRange.second * 180 / M_PI + 0.5 ); // 0.5 is roughly half size of a crystal
   REveGeoShape *sc = fireworks::getShape( "supercluster", sc_box, pb->Collection()->GetMainColor());
   sc->SetPickable( kTRUE );
   pb->SetupAddElement( sc, oItemHolder );
   return true;
}

bool makeRhoZSuperCluster( REveDataProxyBuilderBase* pb,
                           const reco::SuperClusterRef& iCluster,
                           float iPhi,
                           REveElement* oItemHolder )
{   
   if( !iCluster.isAvailable()) return false;

   fireworks::Context* context = fireworks::Context::getInstance();
   //  REveGeoManagerHolder gmgr( REveGeoShape::GetGeoMangeur());
   REveGeoManagerHolder gmgr( REveGeoShape::GetGeoManager());
   double theta_max = 0;
   double theta_min = 10;
   std::vector<std::pair<DetId, float> > detids = iCluster->hitsAndFractions();
   if (detids.empty()) return false;
   for( std::vector<std::pair<DetId, float> >::const_iterator id = detids.begin(), end = detids.end(); id != end; ++id )
   {
      const float* corners = context->getGeom()->getCorners( id->first.rawId());
      if( corners != nullptr )
      {
         std::vector<float> centre( 3, 0 );

         for( unsigned int i = 0; i < 24; i += 3 )
         {	 
            centre[0] += corners[i];
            centre[1] += corners[i + 1];
            centre[2] += corners[i + 2];
         }

         double theta = REveVector( centre[0], centre[1], centre[2] ).Theta();
         if( theta > theta_max ) theta_max = theta;
         if( theta < theta_min ) theta_min = theta;
      }
   }
   // expand theta range by the size of a crystal to avoid segments of zero length
   bool barrel = true; 
   if ((theta_max > 0 && theta_max <  context->caloTransAngle()) || 
       ( theta_min > (TMath::Pi() -context->caloTransAngle())) )
   {
        barrel = false; 
   }
 
   double z_ecal = barrel ? context->caloZ1() : context->caloZ2();
   double r_ecal = barrel ? context->caloR1() : context->caloR2();

   fireworks::addRhoZEnergyProjection( pb, oItemHolder, r_ecal-1, z_ecal-1,
				       theta_min - 0.003, theta_max + 0.003,
				       iPhi );

   return true;
}

}
