#ifndef CANDIDATES_CANDIDATE_UTILS_H
# define CANDIDATES_CANDIDATE_UTILS_H

#include "DataFormats/Candidate/interface/Candidate.h"

namespace ROOT {
namespace Experimental {
class REveTrack;
class REveTrackPropagator;
class REveStraightLineSet;
}
}

namespace fireworks {

   ROOT::Experimental::REveTrack* prepareCandidate( const reco::Candidate& track,
				ROOT::Experimental::REveTrackPropagator* propagator);

   void addStraightLineSegment( ROOT::Experimental::REveStraightLineSet* marker,
				reco::Candidate const* cand,
				double scale_factor = 2);
}

#endif // CANDIDATES_CANDIDATE_UTILS_H
