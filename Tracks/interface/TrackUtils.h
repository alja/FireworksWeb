#ifndef FireworksWeb_Tracks_TrackUtils_h
#define FireworksWeb_Tracks_TrackUtils_h
// -*- C++ -*-
//
// Package:     Tracks
// Class  :     TrackUtils
//

// system include files
#include "ROOT/REveVSDStructs.hxx"
#include <set>

// forward declarations
namespace reco 
{
   class Track;
}
class RecSegment;

namespace ROOT {
namespace Experimental {
class REveElement;
class REveTrack;
class REveTrackPropagator;
class REveStraightLineSet;
}
}

class FWGeometry;

class SiPixelCluster;
class SiStripCluster;
class TrackingRecHit;
class DetId;

using namespace ROOT::Experimental;

namespace fireworks {


struct State {
   REveVector position;
   REveVector momentum;
   bool valid;
   State() : valid(false) {
   }
   State(const REveVector& pos) :
      position(pos), valid(false) {
   }
   State(const REveVector& pos, const REveVector& mom) :
      position(pos), momentum(mom), valid(true) {
   }
};

class StateOrdering {
   REveVector m_direction;
public:
   StateOrdering( const REveVector& momentum ) {
      m_direction = momentum;
      m_direction.Normalize();
   }
   bool operator() ( const State& state1,
                     const State& state2 ) const {
      double product1 = state1.position.Perp()*(state1.position.fX*m_direction.fX + state1.position.fY*m_direction.fY>0 ? 1 : -1);
      double product2 = state2.position.Perp()*(state2.position.fX*m_direction.fX + state2.position.fY*m_direction.fY>0 ? 1 : -1);
      return product1 < product2;
   }
};

REveTrack* prepareTrack( const reco::Track& track,
                         REveTrackPropagator* propagator,
                         const std::vector<REveVector>& extraRefPoints = std::vector<REveVector>() );
 
float pixelLocalX( const double mpx, const float* );
float pixelLocalY( const double mpy, const float* );

float phase2PixelLocalX(const double mpx, const float *, const float *);
float phase2PixelLocalY(const double mpy, const float *, const float *);

void localSiStrip( short strip, float* localTop, float* localBottom, const float* pars, unsigned int id );

// void pushPixelHits( std::vector<TVector3> &pixelPoints, const FWEventItem &iItem, const reco::Track &t );   
// void pushNearbyPixelHits( std::vector<TVector3> &pixelPoints, const FWEventItem &iItem, const reco::Track &t );   
void pushPixelCluster( std::vector<TVector3> &pixelPoints, const FWGeometry &geom, DetId id, const SiPixelCluster &c, const float* pars ); 

// void addSiStripClusters( const FWEventItem* iItem, const reco::Track &t, class REveElement *tList, bool addNearbyClusters, bool master );

// Helpers for data extraction
const SiStripCluster* extractClusterFromsTrackingRecHit( const TrackingRecHit* rh );

// Helper functions to get human readable informationa about given DetId
// (copied from TrackingTools/TrackAssociator)
std::string info( const DetId& );
std::string info( const std::set<DetId>& );
std::string info( const std::vector<DetId>& );
}

#endif // FireworksWeb_Tracks_TrackUtils_h
