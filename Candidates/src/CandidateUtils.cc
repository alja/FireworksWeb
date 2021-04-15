#include "FireworksWeb/Candidates/interface/CandidateUtils.h"
#include "ROOT/REveVSDStructs.hxx"
#include "ROOT/REveTrack.hxx"
#include "ROOT/REveVector.hxx"
#include "ROOT/REveStraightLineSet.hxx"

namespace fireworks {

using namespace ROOT::Experimental;

REveTrack*
prepareCandidate(const reco::Candidate& track,
                 REveTrackPropagator* propagator)
{
   REveRecTrack t;
   t.fBeta = 1.;
   t.fP = REveVector( track.px(), track.py(), track.pz() );
   t.fV = REveVector( track.vertex().x(), track.vertex().y(), track.vertex().z() );
   t.fSign = track.charge();
   REveTrack* trk = new REveTrack(&t, propagator);
   return trk;
}
  
void
addStraightLineSegment( REveStraightLineSet* marker,
                        reco::Candidate const* cand,
                        double scale_factor)
{
   double phi = cand->phi();
   double theta = cand->theta();
   double size = cand->pt() * scale_factor;
   marker->AddLine( 0, 0, 0, size * cos(phi)*sin(theta), size *sin(phi)*sin(theta), size*cos(theta));
}
}
