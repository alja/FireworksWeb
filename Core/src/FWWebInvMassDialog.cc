// -*- C++ -*-
//
// Package:     Core
// Class  :     FWWebInvMassDialog
//
// Implementation:
//     [Notes on implementation]
//
// Original Author:  Matevz Tadel
//         Created:  Mon Nov 22 11:05:57 CET 2010
//

// system include files

// user include files
#include "FireworksWeb/Core/interface/FWWebInvMassDialog.h"
#include "FireworksWeb/Core/interface/FWWebEventItem.h"

#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/TrackReco/interface/TrackBase.h"
#include "DataFormats/Math/interface/deltaR.h"

#include "TClass.h"
#include "TMath.h"
#include "ROOT/REveManager.hxx"

#include "ROOT/REveScene.hxx"
#include "ROOT/REveSelection.hxx"
#include "ROOT/REveDataCollection.hxx"

#include "nlohmann/json.hpp"

using namespace ROOT::Experimental;

FWWebInvMassDialog::FWWebInvMassDialog()
{
  SetTitle("I'm a content of FWWebInvMassDialog");
  SetPickable(false);
}

FWWebInvMassDialog::~FWWebInvMassDialog() {}

void FWWebInvMassDialog::addLine(const TString &line) {
  fTitle += line.Data();
  fTitle += "\n";
}

void FWWebInvMassDialog::Calculate()
{
  fTitle = "<pre>";

  TClass *rc_class = TClass::GetClass(typeid(reco::Candidate));
  TClass *rtb_class = TClass::GetClass(typeid(reco::TrackBase));


 // addLine(TString::Format(" %d items in selection", (int)sted.size()));
  addLine("");
  addLine("--------------------------------------------------+--------------");
  addLine("       px          py          pz          pT     | Collection");
  addLine("--------------------------------------------------+--------------");

  math::XYZVector sum;
  double sum_len = 0;
  double sum_len_xy = 0;
  int n = 0;
  math::XYZVector first, second;

  auto s = ROOT::Experimental::gEve->GetScenes()->FindChild("Collections");
  for (auto &c : s->RefChildren())
  {
    REveDataCollection *coll = (REveDataCollection *)(c);
    auto items = coll->GetItemList();
    for (auto &au : items->RefAunts())
    {
      if (au == gEve->GetSelection())
      {
        //std::cout << c->GetName() << " " << items->GetImpliedSelected() << " --- " << items->RefSelectedSet().size() << "\n";
        for (auto &ss : items->RefSelectedSet())
        {
          TString line;
          TClass *model_class = coll->GetItemClass();
          void *model_data = const_cast<void *>(coll->GetDataPtr(ss));
          math::XYZVector v;
          bool ok_p = false;

          reco::Candidate *rc = reinterpret_cast<reco::Candidate *>(model_class->DynamicCast(rc_class, model_data));

          if (rc != nullptr)
          {
            ok_p = true;
            v.SetXYZ(rc->px(), rc->py(), rc->pz());
          }
          else
          {
            reco::TrackBase *rtb = reinterpret_cast<reco::TrackBase *>(model_class->DynamicCast(rtb_class, model_data));

            if (rtb != nullptr)
            {
              ok_p = true;
              v.SetXYZ(rtb->px(), rtb->py(), rtb->pz());
            }
          }

          if (ok_p)
          {
            sum += v;
            sum_len += TMath::Sqrt(v.mag2());
            sum_len_xy += TMath::Sqrt(v.perp2());

            line = TString::Format("  %+10.3f  %+10.3f  %+10.3f  %10.3f", v.x(), v.y(), v.z(), TMath::Sqrt(v.perp2()));
          }
          else
          {
            line = TString::Format("  -------- not a Candidate or TrackBase --------");
          }
          line += TString::Format("  | %s[%d]", coll->GetCName(), ss);

          addLine(line);

          if (n == 0)
            first = v;
          else if (n == 1)
            second = v;
        }

        break;
      }
    }
  }

  addLine("--------------------------------------------------+--------------");
  addLine(TString::Format(
      "  %+10.3f  %+10.3f  %+10.3f  %10.3f  | Sum", sum.x(), sum.y(), sum.z(), TMath::Sqrt(sum.perp2())));
  addLine("");
  addLine(TString::Format("m  = %10.3f", TMath::Sqrt(TMath::Max(0.0, sum_len * sum_len - sum.mag2()))));
  addLine(TString::Format("mT = %10.3f", TMath::Sqrt(TMath::Max(0.0, sum_len_xy * sum_len_xy - sum.perp2()))));
  addLine(TString::Format("HT = %10.3f", sum_len_xy));

  if (n == 2) {
    addLine(TString::Format("deltaPhi  = %+6.4f", deltaPhi(first.Phi(), second.Phi())));
    addLine(TString::Format("deltaEta  = %+6.4f", first.Eta() - second.Eta()));
    addLine(TString::Format("deltaR    = % 6.4f", deltaR(first.Eta(), first.Phi(), second.Eta(), second.Phi())));
  }

  fTitle += "</pre>";
  StampObjProps();
}

int FWWebInvMassDialog::WriteCoreJson(nlohmann::json &j, int rnr_offset)
{
  using namespace  nlohmann;
  int ret = REveElement::WriteCoreJson(j, rnr_offset);
  j["UT_PostStream"] = "UT_refresh_invmass_dialog";
   
  return ret;
}
