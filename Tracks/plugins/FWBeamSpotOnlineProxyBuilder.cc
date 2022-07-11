/*
 *  FWBeamSpotProxyBuilder.cc
 *  FWorks
 *
 *  Created by Ianna Osborne on 7/29/10.
 *
 */

#include "ROOT/REveDataSimpleProxyBuilderTemplate.hxx"
#include "ROOT/REvePointSet.hxx"
#include "ROOT/REveStraightLineSet.hxx"

#include "DataFormats/Scalers/interface/BeamSpotOnline.h"
#include "FireworksWeb/Core/interface/FWWebEventItem.h"
#include "FireworksWeb/Core/interface/FWProxyBuilderFactory.h"


using namespace ROOT::Experimental;

class FWBeamSpotOnlineProxyBuilder : public REveDataSimpleProxyBuilderTemplate<BeamSpotOnline>
{
public:
   REGISTER_FWPB_METHODS();

private:
   using REveDataSimpleProxyBuilderTemplate<BeamSpotOnline>::BuildItem;
   virtual void BuildItem(const BeamSpotOnline& iData, int /*idx*/, REveElement* oItemHolder, const REveViewContext* vc) override
   {
      REvePointSet* pointSet = new REvePointSet;
      SetupAddElement(pointSet, oItemHolder, true);

      REveStraightLineSet* lineSet = new REveStraightLineSet;
      SetupAddElement(lineSet, oItemHolder, true);

      double posx = iData.x();
      double posy = iData.y();
      double posz = iData.z();
      double errx = iData.err_x();
      double erry = iData.err_y();
      double errz = iData.err_z();

      pointSet->SetNextPoint(posx, posy, posz);
      pointSet->SetNextPoint(posx + errx, posy + erry, posz + errz);
      pointSet->SetNextPoint(posx - errx, posy - erry, posz - errz);

      lineSet->AddLine(posx + errx, posy + erry, posz + errz, posx - errx, posy - erry, posz - errz);
   }
};

REGISTER_FW2PROXYBUILDER(FWBeamSpotOnlineProxyBuilder, BeamSpotOnline, "Beam Spot Online");

/*
class FWBeamSpotOnlineProxyBuilder : public FWSimpleProxyBuilderTemplate<BeamSpotOnline> {
public:
  FWBeamSpotOnlineProxyBuilder(void) {}
  ~FWBeamSpotOnlineProxyBuilder(void) override {}

  REGISTER_PROXYBUILDER_METHODS();

private:
  // Disable default copy constructor
  FWBeamSpotOnlineProxyBuilder(const FWBeamSpotOnlineProxyBuilder&) = delete;
  // Disable default assignment operator
  const FWBeamSpotOnlineProxyBuilder& operator=(const FWBeamSpotOnlineProxyBuilder&) = delete;

  using FWSimpleProxyBuilderTemplate<BeamSpotOnline>::build;
  void build(const BeamSpotOnline& iData, unsigned int iIndex, REveElement& oItemHolder, const FWViewContext*) override;
};

void FWBeamSpotOnlineProxyBuilder::build(const BeamSpotOnline& iData,
                                         unsigned int iIndex,
                                         REveElement& oItemHolder,
                                         const FWViewContext*) {
  REvePointSet* pointSet = new REvePointSet;
  setupAddElement(pointSet, &oItemHolder);

  REveStraightLineSet* lineSet = new REveStraightLineSet;
  setupAddElement(lineSet, &oItemHolder);

  double posx = iData.x();
  double posy = iData.y();
  double posz = iData.z();
  double errx = iData.err_x();
  double erry = iData.err_y();
  double errz = iData.err_z();

  pointSet->SetNextPoint(posx, posy, posz);
  pointSet->SetNextPoint(posx + errx, posy + erry, posz + errz);
  pointSet->SetNextPoint(posx - errx, posy - erry, posz - errz);

  lineSet->AddLine(posx + errx, posy + erry, posz + errz, posx - errx, posy - erry, posz - errz);
}

REGISTER_FWPROXYBUILDER(FWBeamSpotOnlineProxyBuilder,
                        BeamSpotOnline,
                        "Beam Spot Online",
                        FWViewType::kAll3DBits | FWViewType::kAllRPZBits);
*/
