#ifndef FireworksWeb_Calo_makeEveJetCone_h
#define FireworksWeb_Calo_makeEveJetCone_h
// -*- C++ -*-
//
// Package:     Calo
// Class  :     makeEveJetCone
//
/**\class makeEveJetCone makeEveJetCone.h FireworksWeb/Calo/interface/makeEveJetCone.h

   Description: <one line class summary>

   Usage:
    <usage>

 */
//
// Original Author:  Alja Mrak-Tadel
//         Created:  Thu Oct 22 22:59:29 EST 20q0
//

// system include files

// user include files

// forward declarations
namespace ROOT {
namespace Experimental {
class REveJetCone;
}
}

namespace reco {
   class Jet;
}

namespace fireworks {
   class Context;
   ROOT::Experimental::REveJetCone*  makeEveJetCone(const reco::Jet& iData, fireworks::Context* context);
}


#endif
