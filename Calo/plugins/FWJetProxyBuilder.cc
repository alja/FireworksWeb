#ifndef FireworksWeb_Calo_FW2JetProxyBuilder_h
#define FireworksWeb_Calo_FW2JetProxyBuilder_h

#include "ROOT/REveDataSimpleProxyBuilderTemplate.hxx"
#include "ROOT/REveJetCone.hxx"
#include "ROOT/REveScalableStraightLineSet.hxx"
#include "ROOT/REveViewContext.hxx"

#include "FireworksWeb/Core/interface/Context.h"
#include "FireworksWeb/Core/interface/FWViewEnergyScale.h"
#include "FireworksWeb/Calo/interface/makeEveJetCone.h"
#include "FireworksWeb/Core/interface/FWProxyBuilderFactory.h"

#include "DataFormats/JetReco/interface/Jet.h"

using namespace ROOT::Experimental;

class FWJetProxyBuilder: public REveDataSimpleProxyBuilderTemplate<reco::Jet>
{
public:
   REGISTER_FWPB_METHODS();

   using REveDataSimpleProxyBuilderTemplate<reco::Jet>::HaveSingleProduct;
   bool HaveSingleProduct() const override { return false; }

   using REveDataSimpleProxyBuilderTemplate<reco::Jet>::BuildViewType;
  void BuildViewType(const reco::Jet& dj, int idx, REveElement* iItemHolder, const std::string& viewType, const REveViewContext*) override
   {
      fireworks::Context *context = fireworks::Context::getInstance();

      REveJetCone *cone = fireworks::makeEveJetCone(dj, context);
      cone->SetLineColor(iItemHolder->GetMainColor());
      SetupAddElement(cone, iItemHolder, true);

      if (viewType == "RPhi" || viewType == "RhoZ")
      {
         REveVector p1;
         REveVector p2;

         float size = 1.f; // values are saved in scale
         double theta = dj.theta();
         double phi = dj.phi();

         if (viewType == "RhoZ")
         {
            static const float_t offr = 4;
            float r_ecal = context->caloR1() + offr;
            float z_ecal = context->caloZ1() + offr / tan(context->caloTransAngle());
            double r(0);
            if (theta < context->caloTransAngle() || M_PI - theta < context->caloTransAngle())
            {
               z_ecal = context->caloZ2() + offr / tan(context->caloTransAngle());
               r = z_ecal / fabs(cos(theta));
            }
            else
            {
               r = r_ecal / sin(theta);
            }

            p1.Set(0., (phi > 0 ? r * fabs(sin(theta)) : -r * fabs(sin(theta))), r * cos(theta));
            p2.Set(0., (phi > 0 ? (r + size) * fabs(sin(theta)) : -(r + size) * fabs(sin(theta))), (r + size) * cos(theta));
         }
         else if (viewType == "RPhi")
         {
            float ecalR = context->caloR1() + 4;
            p1.Set(ecalR * cos(phi), ecalR * sin(phi), 0);
            p2.Set((ecalR + size) * cos(phi), (ecalR + size) * sin(phi), 0);
         }

         auto marker = new ROOT::Experimental::REveScalableStraightLineSet("jetline");
         marker->SetScaleCenter(p1.fX, p1.fY, p1.fZ);
         marker->AddLine(p1, p2);

         marker->SetScale(dj.et() * 2); // TODO :: implement scales

         marker->SetLineWidth(4);
         SetupAddElement(marker, iItemHolder, true);
      }
      fireworks::Context::getInstance()->voteMaxEtAndEnergy(dj.et(), dj.energy());
   }

   using REveDataProxyBuilderBase::LocalModelChanges;
   void LocalModelChanges(int idx, REveElement* el, const REveViewContext* ctx) override
   {
      if (el->HasChildren())
      {
      REveJetCone* cone = dynamic_cast<REveJetCone*>(el->FirstChild());
      cone->SetLineColor(cone->GetMainColor());
      }
   }

   void ScaleProduct(REveElement *parent, const std::string &vtype) override
   {
      int idx = 0;
      for (auto &holder : parent->RefChildren())
      {
         if (holder->NumChildren() == 2)
         {
            reco::Jet *jet = (reco::Jet *)Collection()->GetDataPtr(idx);
            auto energyScale = fireworks::Context::getInstance()->energyScale();
            float value = energyScale->getPlotEt() ? jet->et() : jet->energy();
            REveScalableStraightLineSet *line = dynamic_cast<REveScalableStraightLineSet *>(holder->LastChild());
            line->SetScale(energyScale->getScaleFactor3D() * value);
            line->StampObjProps();
            for (auto &prj : line->RefProjecteds())
               prj->UpdateProjection();
         }
         idx++;
      }
   }
};

REGISTER_FW2PROXYBUILDER(FWJetProxyBuilder, reco::Jet, "Jets");
#endif
