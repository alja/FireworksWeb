#ifndef FireworksWeb_Conversions_FW2ConvProxyBuilder_h
#define FireworksWeb_Conversions_FW2ConvProxyBuilder_h


// -*- C++ -*-
//
// Package:     Conversions
// Class  :     FWConversionProxyBuilder
//
// Implementation:
//     <Notes on implementation>
//
// Original Author:  Chris Jones
//         Created:  Tue Dec  2 14:17:03 EST 2008
//
#include "ROOT/REveCompound.hxx"
#include "ROOT/REveLine.hxx"
#include "ROOT/REveScalableStraightLineSet.hxx"

#include "ROOT/REveDataSimpleProxyBuilderTemplate.hxx"

#include "FireworksWeb/Core/interface/Context.h"

#include "FireworksWeb/Tracks/interface/TrackUtils.h"
#include "FireworksWeb/Electrons/interface/makeSuperCluster.h"
#include "FireworksWeb/Candidates/interface/CandidateUtils.h"
#include "FireworksWeb/Core/interface/FWProxyBuilderFactory.h"

#include "DataFormats/EgammaCandidates/interface/Conversion.h"
#include "DataFormats/GsfTrackReco/interface/GsfTrack.h"

////////////////////////////////////////////////////////////////////////////////
//
//   3D and RPZ proxy builder with shared track list
//
////////////////////////////////////////////////////////////////////////////////

using namespace ROOT::Experimental;

class FWConversionProxyBuilder : public REveDataSimpleProxyBuilderTemplate<reco::Conversion>
{
public:
  REGISTER_FWPB_METHODS();
  virtual bool HaveSingleProduct() const { return false; }

  using REveDataSimpleProxyBuilderTemplate<reco::Conversion>::BuildItemViewType;
  virtual void BuildItemViewType(const reco::Conversion &conversion, int /*idx*/, REveElement *oItemHolder, const std::string &type, const REveViewContext *) override
  {
    printf("Build conversion \n");
    REveLine *line = new REveLine(0);
    if (conversion.nTracks() == 2)
    {
      if (fabs(conversion.zOfPrimaryVertexFromTracks()) < fireworks::Context::caloZ1())
        line->SetNextPoint(0., 0., conversion.zOfPrimaryVertexFromTracks());
      else
        line->SetNextPoint(0., 0., 0.);

      float phi = conversion.pairMomentum().phi();
      if (fabs(conversion.pairMomentum().eta()) < fireworks::Context::caloTransEta())
      {
        float radius = fireworks::Context::caloR1();
        float z = radius / tan(conversion.pairMomentum().theta());
        line->SetNextPoint(radius * cos(phi), radius * sin(phi), z);
      }
      else
      {
        float z = fireworks::Context::caloZ1();
        float radius = z * tan(conversion.pairMomentum().theta());
        z *= (conversion.pairMomentum().eta() / fabs(conversion.pairMomentum().eta()));
        line->SetNextPoint(radius * cos(phi), radius * sin(phi), z);
      }
    }
    else
    {
      line->SetNextPoint(0., 0., 0.);
      line->SetNextPoint(0., 0., 0.);
    }

    REveVector bvec = line->GetLineStart();
    REveVector evec = line->GetLineEnd();
    if (bvec.Mag() != evec.Mag())
    {
      SetupAddElement(line, oItemHolder);
    }
    else
    {
      printf("Conversion line size 0 \n");
      line->CheckReferenceCount();
    }
  }
};

REGISTER_FW2PROXYBUILDER(FWConversionProxyBuilder, reco::Conversion, "Conversions");
#endif
