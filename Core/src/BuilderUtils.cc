#include "FireworksWeb/Core/interface/BuilderUtils.h"
#include "FireworksWeb/Core/interface/Context.h"
#include "FireworksWeb/Core/interface/fwLog.h"

#include "FWCore/Common/interface/EventBase.h"

#include "TGeoBBox.h"
#include "TColor.h"
#include "TROOT.h"
//#include "REveBox.h"
#include "ROOT/REveDataProxyBuilderBase.hxx"
#include "ROOT/REveScalableStraightLineSet.hxx"
#include "ROOT/REveStraightLineSet.hxx"
#include "ROOT/REveTrans.hxx"
#include "ROOT/REveGeoShape.hxx"

#include <cmath>
#include <ctime>

using namespace ROOT::Experimental;

namespace fireworks
{
   std::pair<double,double> getPhiRange( const std::vector<double>& phis, double phi )
   {
      double min =  100;
      double max = -100;

      for( std::vector<double>::const_iterator i = phis.begin();
           i != phis.end(); ++i )
      {
         double aphi = *i;
         // make phi continuous around jet phi
         if( aphi - phi > M_PI ) aphi -= 2*M_PI;
         if( phi - aphi > M_PI ) aphi += 2*M_PI;
         if( aphi > max ) max = aphi;
         if( aphi < min ) min = aphi;
      }

      if( min > max ) return std::pair<double,double>( 0, 0 );

      return std::pair<double,double>( min, max );
   }

   REveGeoShape* getShape( const char* name,
                           TGeoBBox* shape,
                           Color_t color )
   {
      REveGeoShape* egs = new REveGeoShape( name );
      TColor* c = gROOT->GetColor( color );
      Float_t rgba[4] = { 1, 0, 0, 1 };
      if( c )
      {
         rgba[0] = c->GetRed();
         rgba[1] = c->GetGreen();
         rgba[2] = c->GetBlue();
      }
      egs->SetMainColorRGB( rgba[0], rgba[1], rgba[2] );
      egs->SetShape( shape );
      return egs;
   }

   void addRhoZEnergyProjection( REveDataProxyBuilderBase* pb, REveElement* container,
                                 double r_ecal, double z_ecal,
                                 double theta_min, double theta_max,
                                 double phi )
   {
      REveGeoManagerHolder gmgr( REveGeoShape::GetGeoManager());
      double z1 = r_ecal / tan( theta_min );
      if( z1 > z_ecal ) z1 = z_ecal;
      if( z1 < -z_ecal ) z1 = -z_ecal;
      double z2 = r_ecal / tan( theta_max );
      if( z2 > z_ecal ) z2 = z_ecal;
      if( z2 < -z_ecal ) z2 = -z_ecal;
      double r1 = z_ecal * fabs( tan( theta_min ));
      if( r1 > r_ecal ) r1 = r_ecal;
      if( phi < 0 ) r1 = -r1;
      double r2 = z_ecal * fabs( tan( theta_max ));
      if( r2 > r_ecal ) r2 = r_ecal;
      if( phi < 0 ) r2 = -r2;

      if( fabs(r2 - r1) > 1 )
      {
         TGeoBBox *sc_box = new TGeoBBox( 0., fabs( r2 - r1 ) / 2, 1 );
         REveGeoShape *element = new REveGeoShape("r-segment");
         element->SetShape(sc_box);
         REveTrans &t = element->RefMainTrans();
         t(1,4) = 0;
         t(2,4) = (r2+r1)/2;
         t(3,4) = fabs(z2)>fabs(z1) ? z2 : z1;
         pb->SetupAddElement(element, container);
      }
      if( fabs(z2 - z1) > 1 )
      {
         TGeoBBox *sc_box = new TGeoBBox( 0., 1, ( z2 - z1 ) / 2 );
         REveGeoShape *element = new REveGeoShape("z-segment");
         element->SetShape( sc_box );
         REveTrans &t = element->RefMainTrans();
         t(1,4) = 0;
         t(2,4) = fabs(r2)>fabs(r1) ? r2 : r1;
         t(3,4) = (z2+z1)/2;
         pb->SetupAddElement(element, container);
      }
   }

   std::string getTimeGMT( const edm::EventBase& event )
   {
      time_t t( event.time().value() >> 32 );
      std::string text( asctime( gmtime( &t ) ) );
      size_t pos = text.find( '\n' );
      if( pos != std::string::npos ) text = text.substr( 0, pos );
      text += " GMT";
      return text;
   }

   std::string getLocalTime( const edm::EventBase& event )
   {

      time_t t( event.time().value() >> 32 );
      struct tm * xx =  localtime( &t );
      std::string text( asctime(xx) );
      try {
      size_t pos = text.find('\n');
      if( pos != std::string::npos ) text = text.substr( 0, pos );
      text += " ";
      if( xx->tm_isdst )
         text += "EDT"; //tzname[1];
      else
         text += "EST"; //tzname[0];
      }
      catch (std::exception& e) {
         fwLog(fwlog::kError) << "getLocalTime() " << e.what() << std::endl;
      }
      return text;
   }

   void invertBox( std::vector<float> &corners )
   {
      std::swap( corners[0], corners[9] );
      std::swap( corners[1], corners[10] );
      std::swap( corners[2], corners[11] );

      std::swap( corners[3], corners[6] );
      std::swap( corners[4], corners[7] );
      std::swap( corners[5], corners[8] );

      std::swap( corners[12], corners[21] );
      std::swap( corners[13], corners[22] );
      std::swap( corners[14], corners[23] );

      std::swap( corners[15], corners[18] );
      std::swap( corners[16], corners[19] );
      std::swap( corners[17], corners[20] );
   }

   void addBox( const std::vector<float> &corners, REveElement* comp, REveDataProxyBuilderBase* pb )
   {/*
      REveBox* eveBox = new REveBox( "Box" );
      eveBox->SetDrawFrame( false );
      eveBox->SetPickable( true );
      eveBox->SetVertices( &corners[0] );

      pb->SetupAddElement( eveBox, comp );
    */
      std::cerr << "fireworks::addBox not implemented \n";
   }

   void addCircle( double eta, double phi, double radius, const unsigned int nLineSegments, REveElement* comp, REveDataProxyBuilderBase* pb )
   {
      REveStraightLineSet* container = new REveStraightLineSet;

      for( unsigned int iphi = 0; iphi < nLineSegments; ++iphi )
      {
         container->AddLine( eta + radius * cos( 2 * M_PI / nLineSegments * iphi ),
                             phi + radius * sin( 2 * M_PI / nLineSegments * iphi ),
                             0.01,
                             eta + radius * cos( 2 * M_PI / nLineSegments * ( iphi + 1 )),
                             phi + radius * sin( 2 * M_PI / nLineSegments * ( iphi + 1 )),
                             0.01 );
      }
      pb->SetupAddElement( container, comp );
   }

   void addDashedArrow( double phi, double size, REveElement* comp, REveDataProxyBuilderBase* pb )
   {
      REveScalableStraightLineSet* marker = new REveScalableStraightLineSet;
      marker->SetLineWidth( 1 );
      marker->SetLineStyle( 2 );
      marker->AddLine( 0, 0, 0, size * cos( phi ), size * sin( phi ), 0 );
      marker->AddLine( size * 0.9 * cos( phi + 0.03 ), size * 0.9 * sin( phi + 0.03 ), 0, size * cos( phi ), size * sin( phi ), 0 );
      marker->AddLine( size * 0.9 * cos( phi - 0.03 ), size * 0.9 * sin( phi - 0.03 ), 0, size * cos( phi ), size * sin( phi ), 0 );
      pb->SetupAddElement( marker, comp );
   }

   void addDashedLine( double phi, double theta, double size, REveElement* comp, REveDataProxyBuilderBase* pb )
   {
      double r( 0 );
      fireworks::Context* ctx = fireworks::Context::getInstance();
      if( theta < ctx->caloTransAngle() || M_PI - theta < ctx->caloTransAngle())
         r = ctx->caloZ2() / fabs( cos( theta ));
      else
         r = ctx->caloR1() / sin( theta );

      REveStraightLineSet* marker = new REveStraightLineSet;
      marker->SetLineWidth( 2 );
      marker->SetLineStyle( 2 );
      marker->AddLine( r * cos( phi ) * sin( theta ), r * sin( phi ) * sin( theta ), r * cos( theta ),
                       ( r + size ) * cos( phi ) * sin( theta ), ( r + size ) * sin( phi ) * sin( theta ), ( r + size ) * cos( theta ));
      pb->SetupAddElement( marker, comp );
   }

   void addDoubleLines( double phi, REveElement* comp, REveDataProxyBuilderBase* pb )
   {
      REveStraightLineSet* mainLine = new REveStraightLineSet;
      mainLine->AddLine( -5.191, phi, 0.01, 5.191, phi, 0.01 );
      pb->SetupAddElement( mainLine, comp );

      phi = phi > 0 ? phi - M_PI : phi + M_PI;
      REveStraightLineSet* secondLine = new REveStraightLineSet;
      secondLine->SetLineStyle( 7 );
      secondLine->AddLine( -5.191, phi, 0.01, 5.191, phi, 0.01 );
      pb->SetupAddElement( secondLine, comp );
   }

   //______________________________________________________________________________
   void energyScaledBox3DCorners( const float* corners, float scale, std::vector<float>& scaledCorners, bool invert)
   {
      std::vector<float> centre( 3, 0 );

      for( unsigned int i = 0; i < 24; i += 3 )
      {
         centre[0] += corners[i];
         centre[1] += corners[i + 1];
         centre[2] += corners[i + 2];
      }

      for( unsigned int i = 0; i < 3; ++i )
         centre[i] *= 1.0f / 8.0f;

      // Coordinates for a scaled version of the original box
      for( unsigned int i = 0; i < 24; i += 3 )
      {
         scaledCorners[i] = centre[0] + ( corners[i] - centre[0] ) * scale;
         scaledCorners[i + 1] = centre[1] + ( corners[i + 1] - centre[1] ) * scale;
         scaledCorners[i + 2] = centre[2] + ( corners[i + 2] - centre[2] ) * scale;
      }

      if( invert )
         invertBox( scaledCorners );
   }

   void drawEnergyScaledBox3D( const float* corners, float scale, REveElement* comp, REveDataProxyBuilderBase* pb, bool invert )
   {
      std::vector<float> scaledCorners( 24 );
      energyScaledBox3DCorners(corners, scale, scaledCorners, invert);
      addBox( scaledCorners, comp, pb );
   }
   //______________________________________________________________________________

   void etScaledBox3DCorners( const float* corners, float energy, float maxEnergy, std::vector<float>& scaledCorners, bool invert)
   {
      std::vector<float> centre( 3, 0 );

      for( unsigned int i = 0; i < 24; i += 3 )
      {
         centre[0] += corners[i];
         centre[1] += corners[i + 1];
         centre[2] += corners[i + 2];
      }

      for( unsigned int i = 0; i < 3; ++i )
         centre[i] *= 1.0f / 8.0f;

      REveVector c( centre[0], centre[1], centre[2] );
      float scale = energy / maxEnergy * sin( c.Theta());

      // Coordinates for a scaled version of the original box
      for( unsigned int i = 0; i < 24; i += 3 )
      {
         scaledCorners[i] = centre[0] + ( corners[i] - centre[0] ) * scale;
         scaledCorners[i + 1] = centre[1] + ( corners[i + 1] - centre[1] ) * scale;
         scaledCorners[i + 2] = centre[2] + ( corners[i + 2] - centre[2] ) * scale;
      }

      if( invert )
         invertBox( scaledCorners );
   }

   void drawEtScaledBox3D( const float* corners, float energy, float maxEnergy, REveElement* comp, REveDataProxyBuilderBase* pb, bool invert )
   {
      std::vector<float> scaledCorners( 24 );
      etScaledBox3DCorners(corners, energy, maxEnergy, scaledCorners, invert);
      addBox( scaledCorners, comp, pb );
   }

   //______________________________________________________________________________
   void energyTower3DCorners( const float* corners, float scale,  std::vector<float>& scaledCorners, bool reflect)
   {
      for( int i = 0; i < 24; ++i )
         scaledCorners[i] = corners[i];
      // Coordinates of a front face scaled
      if( reflect )
      {
         // We know, that an ES rechit geometry in -Z needs correction.
         // The back face is actually its front face.
         for( unsigned int i = 0; i < 12; i += 3 )
         {
            REveVector diff( corners[i] - corners[i + 12], corners[i + 1] - corners[i + 13], corners[i + 2] - corners[i + 14] );
            diff.Normalize();
            diff *= scale;

            scaledCorners[i] = corners[i] + diff.fX;
            scaledCorners[i + 1] = corners[i + 1] + diff.fY;
            scaledCorners[i + 2] = corners[i + 2] + diff.fZ;
         }
      }
      else
      {
         for( unsigned int i = 0; i < 12; i += 3 )
         {
            REveVector diff( corners[i + 12] - corners[i], corners[i + 13] - corners[i + 1], corners[i + 14] - corners[i + 2] );
            diff.Normalize();
            diff *= scale;

            scaledCorners[i] = corners[i + 12];
            scaledCorners[i + 1] = corners[i + 13];
            scaledCorners[i + 2] = corners[i + 14];

            scaledCorners[i + 12] = corners[i + 12] + diff.fX;
            scaledCorners[i + 13] = corners[i + 13] + diff.fY;
            scaledCorners[i + 14] = corners[i + 14] + diff.fZ;
         }
      }
   }

   void drawEnergyTower3D( const float* corners, float scale, REveElement* comp, REveDataProxyBuilderBase* pb, bool reflect )
   {
      std::vector<float> scaledCorners( 24 );
      energyTower3DCorners(corners, scale, scaledCorners, reflect);
      addBox( scaledCorners, comp, pb );
   }

   //______________________________________________________________________________

   void etTower3DCorners( const float* corners, float scale,  std::vector<float>& scaledCorners, bool reflect)
   {
      for( int i = 0; i < 24; ++i )
         scaledCorners[i] = corners[i];
      // Coordinates of a front face scaled
      if( reflect )
      {
         // We know, that an ES rechit geometry in -Z needs correction.
         // The back face is actually its front face.
         for( unsigned int i = 0; i < 12; i += 3 )
         {
            REveVector diff( corners[i] - corners[i + 12], corners[i + 1] - corners[i + 13], corners[i + 2] - corners[i + 14] );
            diff.Normalize();
            diff *= ( scale * sin( diff.Theta()));

            scaledCorners[i] = corners[i] + diff.fX;
            scaledCorners[i + 1] = corners[i + 1] + diff.fY;
            scaledCorners[i + 2] = corners[i + 2] + diff.fZ;
         }
      }
      else
      {
         for( unsigned int i = 0; i < 12; i += 3 )
         {
            REveVector diff( corners[i + 12] - corners[i], corners[i + 13] - corners[i + 1], corners[i + 14] - corners[i + 2] );
            diff.Normalize();
            diff *= ( scale * sin( diff.Theta()));

            scaledCorners[i] = corners[i + 12];
            scaledCorners[i + 1] = corners[i + 13];
            scaledCorners[i + 2] = corners[i + 14];

            scaledCorners[i + 12] = corners[i + 12] + diff.fX;
            scaledCorners[i + 13] = corners[i + 13] + diff.fY;
            scaledCorners[i + 14] = corners[i + 14] + diff.fZ;
         }
      }
   }


   void drawEtTower3D( const float* corners, float scale, REveElement* comp, REveDataProxyBuilderBase* pb, bool reflect )
   {
      std::vector<float> scaledCorners( 24 );
      etTower3DCorners(corners, scale, scaledCorners, reflect);
      addBox( scaledCorners, comp, pb );
   }


} // namespace fireworks
