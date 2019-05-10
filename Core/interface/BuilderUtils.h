#ifndef Fireworks2_Core_BuilderUtils_h
#define Fireworks2_Core_BuilderUtils_h

#include <vector>
#include <string>
#include "Rtypes.h"

namespace ROOT {
namespace Experimental {
class REveGeoShape;
class REveElement;
class REveDataProxyBuilderBase;
}
}

class TGeoBBox;
namespace edm {
   class EventBase;
}

namespace fireworks
{
   std::pair<double,double> getPhiRange( const std::vector<double>& phis,
                                         double phi );
   ROOT::Experimental::REveGeoShape* getShape( const char* name,
                           TGeoBBox* shape,
                           Color_t color );

   void addRhoZEnergyProjection( ROOT::Experimental::REveDataProxyBuilderBase*, ROOT::Experimental::REveElement*,
                                 double r_ecal, double z_ecal,
                                 double theta_min, double theta_max,
                                 double phi );

   std::string getTimeGMT( const edm::EventBase& event );
   std::string getLocalTime( const edm::EventBase& event );

   void invertBox( std::vector<float> &corners );
   void addBox( const std::vector<float> &corners, ROOT::Experimental::REveElement*,  ROOT::Experimental::REveDataProxyBuilderBase*);
   void addCircle( double eta, double phi, double radius, const unsigned int nLineSegments, ROOT::Experimental::REveElement* comp, ROOT::Experimental::REveDataProxyBuilderBase* pb );
   void addDashedArrow( double phi, double size, ROOT::Experimental::REveElement* comp, ROOT::Experimental::REveDataProxyBuilderBase* pb );
   void addDashedLine( double phi, double theta, double size, ROOT::Experimental::REveElement* comp, ROOT::Experimental::REveDataProxyBuilderBase* pb );
   void addDoubleLines( double phi, ROOT::Experimental::REveElement* comp, ROOT::Experimental::REveDataProxyBuilderBase* pb );

   //
   //  box-utilts
   // 
   void energyScaledBox3DCorners( const float* corners, float scale, std::vector<float>&, bool invert = false);
   void drawEnergyScaledBox3D   ( const float* corners, float scale, ROOT::Experimental::REveElement*,  ROOT::Experimental::REveDataProxyBuilderBase*, bool invert = false );

   void energyTower3DCorners( const float* corners, float scale, std::vector<float>&, bool reflect = false);
   void drawEnergyTower3D   ( const float* corners, float scale, ROOT::Experimental::REveElement*, ROOT::Experimental::REveDataProxyBuilderBase*, bool reflect = false );
 
   // AMT: is this needed ?
   void etScaledBox3DCorners( const float* corners, float energy, float maxEnergy,  std::vector<float>& scaledCorners, bool reflect = false );
   void drawEtScaledBox3D( const float* corners, float energy, float maxEnergy, ROOT::Experimental::REveElement*,  ROOT::Experimental::REveDataProxyBuilderBase*, bool reflect = false );
  
   void etTower3DCorners( const float* corners, float scale, std::vector<float>&, bool reflect = false);
   void drawEtTower3D( const float* corners, float scale, ROOT::Experimental::REveElement*, ROOT::Experimental::REveDataProxyBuilderBase*, bool reflect = false );
}

#endif // Fireworks2_Core_BuilderUtils_h
