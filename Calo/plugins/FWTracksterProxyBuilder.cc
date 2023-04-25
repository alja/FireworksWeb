#include "ROOT/REveDataSimpleProxyBuilderTemplate.hxx"
#include "ROOT/REveViewContext.hxx"

#include "TGeoSphere.h"
#include "ROOT/REveTrans.hxx"
#include "ROOT/REveGeoShape.hxx"

#include "FireworksWeb/Core/interface/FWProxyBuilderFactory.h"

#include "DataFormats/HGCalReco/interface/Trackster.h"



using namespace ROOT::Experimental;

class FWTracksterProxyBuilder: public REveDataSimpleProxyBuilderTemplate<ticl::Trackster>
{
public:
   REGISTER_FWPB_METHODS();

   using REveDataSimpleProxyBuilderTemplate<ticl::Trackster>::BuildItem;
   void BuildItem(const ticl::Trackster& iData, int idx, REveElement* iItemHolder, const REveViewContext*) override
   {
      const ticl::Trackster &trackster = iData;
      const ticl::Trackster::Vector &barycenter = trackster.barycenter();
      const std::array<float, 3> &eigenvalues = trackster.eigenvalues();
      const double theta = barycenter.Theta();
      const double phi = barycenter.Phi();

      auto eveEllipsoid = new REveGeoShape("Ellipsoid");
      auto sphere = new TGeoSphere(0., 1.);
      eveEllipsoid->SetShape(sphere);
      eveEllipsoid->InitMainTrans();
      eveEllipsoid->RefMainTrans().Move3PF(barycenter.x(), barycenter.y(), barycenter.z());
      eveEllipsoid->RefMainTrans().SetRotByAnyAngles(theta, phi, 0., "xzy");
      if (eigenvalues[2] > 0 && eigenvalues[1] && eigenvalues[0])
        eveEllipsoid->RefMainTrans().SetScale(sqrt(eigenvalues[2]), sqrt(eigenvalues[1]), sqrt(eigenvalues[0]));
      else
        printf("FWTracksterProxyBuilder error %s idx %d negative eigenvalues!\n", Collection()->GetCName(), idx);

      SetupAddElement(eveEllipsoid, iItemHolder);
   }
};

REGISTER_FW2PROXYBUILDER(FWTracksterProxyBuilder, ticl::Trackster, "Trackster");
