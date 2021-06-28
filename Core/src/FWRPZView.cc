
#include "FireworksWeb/Core/interface/FWRPZView.h"

#include <ROOT/REveManager.hxx>
#include <ROOT/REveProjectionManager.hxx>
#include <ROOT/REveProjectionBases.hxx>

using namespace ROOT::Experimental;

FWRPZView::FWRPZView(std::string vtype):
FWEveView(vtype)
{
  REveProjection::EPType_e projType = (vtype == "RhoZ") ? REveProjection::kPT_RhoZ : REveProjection::kPT_RPhi;
  m_projMgr = new REveProjectionManager(projType);
  m_projMgr->IncDenyDestroy();
  m_projMgr->SetImportEmpty(kTRUE);
}

FWRPZView::~FWRPZView(){}

void
FWRPZView::eventBegin() {

}

void
FWRPZView::eventEnd() {
  FWEveView::eventEnd();
}

void 
FWRPZView::importElements(REveElement* iChildren, float layer, REveElement *iProjectedParent)
{
  m_projMgr->SetCurrentDepth(layer);
  m_projMgr->ImportElements(iChildren, iProjectedParent);
}
