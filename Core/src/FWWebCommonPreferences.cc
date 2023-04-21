
#include "FireworksWeb/Core/interface/FWWebCommonPreferences.h"

#include "ROOT/REveViewer.hxx"
#include "ROOT/REveManager.hxx"
#include "nlohmann/json.hpp"


using namespace ROOT::Experimental;


FW2CommonPref::FW2CommonPref(){}




int FW2CommonPref::WriteCoreJson(nlohmann::json &j, int rnr_offset)
{
  int ret = REveElement::WriteCoreJson(j, rnr_offset);
  j["blacBg"] = false;
  return ret;
}

void FW2CommonPref::setBackground(bool isBlack)
{
    m_blackBg = isBlack;

    for (auto &e : gEve->GetViewers()->RefChildren())
    {
        REveViewer* v = dynamic_cast<REveViewer*>(e);
        v->SetBlackBackground(isBlack);
    }
}
