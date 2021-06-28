#include "FireworksWeb/Core/interface/FWEveView.h"
#include "FireworksWeb/Core/interface/Context.h"

#include <ROOT/REveManager.hxx>
#include <ROOT/REveViewer.hxx>
#include <ROOT/REveScene.hxx>

using namespace ROOT::Experimental;

FWEveView::FWEveView(std::string vtype)
{
    if (vtype.empty())
    {
        m_viewer = dynamic_cast<REveViewer*>(gEve->GetViewers()->FirstChild());
        m_eventScene = gEve->GetEventScene();
        m_geoScene = gEve->GetGlobalScene();
    }
    else
    {
      m_eventScene = gEve->SpawnNewScene(vtype.c_str(), vtype.c_str());
      m_viewer = gEve->SpawnNewViewer(vtype.c_str(), Form("%s View", vtype.c_str()));
      m_viewer->AddScene(m_eventScene);

      // create geometry scene for non table views
      if (vtype != "Table")
      {
        m_geoScene = gEve->SpawnNewScene(Form("GeoScene %s", vtype.c_str()));
        m_viewer->AddScene(m_geoScene);
      }
    }
}

FWEveView::~FWEveView()
{
}

void
FWEveView::eventBegin(){}

void
FWEveView::eventEnd(){}

fireworks::Context*
FWEveView::context(){
  return fireworks::Context::getInstance();
}
