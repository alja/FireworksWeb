#include <iostream>
#include "FireworksWeb/Core/interface/FWWebGeoTableView.h"
#include "FireworksWeb/Core/interface/fwLog.h"
#include "FireworksWeb/Core/interface/Context.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "TGeoManager.h"
#include <ROOT/REveGeoTopNode.hxx>
#include <ROOT/REveViewer.hxx>
#include <ROOT/REveScene.hxx>
#include <ROOT/REveSceneInfo.hxx>
#include <ROOT/REveManager.hxx>
#include <TGeoNode.h>
#include "nlohmann/json.hpp"

using namespace ROOT::Experimental;


void RecursiveSearch(TGeoNode* node, const std::string& target, std::string currentPath, std::string& result) {
    if (!node || !result.empty()) return;

    // Append current node to path

    if (node != gGeoManager->GetTopNode())
    currentPath += "/" + std::string(node->GetName());

    if (target == node->GetName()) {
        result = currentPath;
        return;
    }

    // Iterate through daughters
    TGeoVolume* vol = node->GetVolume();
    int nDaughters = vol->GetNdaughters();
    for (int i = 0; i < nDaughters; ++i) {
        RecursiveSearch(vol->GetNode(i), target, currentPath, result);
    }
}


FWWebGeoTableView::FWWebGeoTableView(const std::string type): FWEveView(type)
{
    try
    {
        TGeoManager* oldm = gGeoManager;
        gGeoManager = nullptr;
        const std::string& path = fireworks::Context::getInstance()->getSimGeomPath();
        auto data = new REveGeoTopNodeData(path.c_str());
        std::string foundPath;

        // for the moment top node is tracker at the startup
        RecursiveSearch(gGeoManager->GetTopNode(), "tracker:Tracker_1", "", foundPath);
        fwLog(fwlog::kInfo) <<  "FWWebGeoTableView locating tracker path " << foundPath << std::endl;
        data->InitPath(foundPath);

        data->RefDescription().SetVisLevel(2);
        eventScene()->AddElement(data);

        // 3D EveViz representation
        auto geoViz = new REveGeoTopNodeViz();
        geoViz->SetGeoData(data);
        geoViz->SetPickable(true);
        data->AddNiece(geoViz);

        // find 3D view
        auto vl = ROOT::Experimental::gEve->GetViewers()->FindChild("3D");
        REveViewer* reveView = dynamic_cast<REveViewer*>(vl);
        REveSceneInfo* si = dynamic_cast<REveSceneInfo*>(reveView->LastChild()); // last scene inf osupposed to be geoscenenfo
        REveScene* geoScene = si->GetScene();
        geoScene->AddElement(geoViz);

        gGeoManager = oldm;
    }
    catch (const std::runtime_error& e) {
       fwLog(fwlog::kInfo) << "FWWebGeoTableView::FWWebGeoTableView " <<  e.what() << "\n";
    }
}


//------------------------------------------------------------------------
/*
int FWWebGeoTableView::WriteCoreJson(nlohmann::json &j, int rnr_offset)
{
}
*/
