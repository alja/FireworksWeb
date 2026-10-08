#ifndef FireworksWeb_Calo_hgcal_gradient_h
#define FireworksWeb_Calo_hgcal_gradient_h

#define FWHGCAL_GRADIENT_STEPS 9

 #include <stdint.h>
 #include "DataFormats/DetId/interface/DetId.h"
 #include <unordered_map>

#include "DataFormats/HGCRecHit/interface/HGCRecHit.h"

 namespace edm {
    class EventBase;
 }

 namespace ROOT { namespace Experimental {
 class REveDataCollection;
 }}
 

namespace fwhgcal
{
   extern const uint8_t gradient[][FWHGCAL_GRADIENT_STEPS];
   typedef std::unordered_map<DetId, const HGCRecHit*> FWHgcalHitmap_t;

   void fillDetIdToHitMap(FWHgcalHitmap_t&);

   void initProxyParameters(ROOT::Experimental::REveDataCollection*);
}

#endif
