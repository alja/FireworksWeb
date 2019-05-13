#ifndef Fireworks2_Electrons_makeSuperCluster_h
#define Fireworks2_Electrons_makeSuperCluster_h

// system include files
#include "DataFormats/EgammaReco/interface/SuperCluster.h"
#include "DataFormats/EgammaReco/interface/SuperClusterFwd.h"

// user include files

// forward declarations
namespace ROOT {
namespace Experimental {
class REveElement;
class REveDataProxyBuilderBase;
}
}

namespace fireworks {
bool makeRhoPhiSuperCluster(ROOT::Experimental::REveDataProxyBuilderBase*,
                               const reco::SuperClusterRef& iCluster,
                               float iPhi,
                               ROOT::Experimental::REveElement* oItemHolder);
   bool makeRhoZSuperCluster(ROOT::Experimental::REveDataProxyBuilderBase*,
                             const reco::SuperClusterRef& iCluster,
                             float iPhi,
                             ROOT::Experimental::REveElement* oItemHolder);
}
#endif
