#include <cstring>
#include <memory>
#include "FireworksWeb/Core/interface/FWWebGeoOverlapTableView.h"
#include "FireworksWeb/Core/interface/fwLog.h"
#include "FireworksWeb/Core/interface/Context.h"
#include "TFile.h"
#include "TKey.h"
#include "TGeoManager.h"
#include <ROOT/REveGeoOverlaps.hxx>
#include <ROOT/REveManager.hxx>
#include <ROOT/REveScene.hxx>

using namespace ROOT::Experimental;

namespace {
   TGeoManager* loadGeometry(const std::string& fname)
   {
      std::unique_ptr<TFile> f(TFile::Open(fname.c_str()));
      if (!f || f->IsZombie())
         throw std::runtime_error("can't open " + fname);

      TIter next(f->GetListOfKeys());
      while (auto* key = (TKey*)next())
         if (!std::strcmp(key->GetClassName(), "TGeoManager"))
            return dynamic_cast<TGeoManager*>(key->ReadObj());

      throw std::runtime_error("no TGeoManager in " + fname);
   }
}

FWWebGeoOverlapTableView::FWWebGeoOverlapTableView(const std::string type): FWEveView(type)
{
    // reading TGeoManager deletes and replaces gGeoManager, keep the one in use
    TGeoManager* oldm = gGeoManager;
    gGeoManager = nullptr;
    try
    {
        const std::string& path = fireworks::Context::getInstance()->getSimGeomPath();
        m_geoManager = loadGeometry(path);

        // the scan runs once here, before the client threads are serving
        double precision = 0.001;
        m_table = new REveGeoOverlapTable("Overlaps", "Overlaps and extrusions");
        fwLog(fwlog::kInfo) << "FWWebGeoOverlapTableView checking overlaps in " << path << ", precision " << precision << " cm\n";
        m_table->ScanOverlaps(m_geoManager, precision);
        fwLog(fwlog::kInfo) << "FWWebGeoOverlapTableView found " << m_table->GetNumOverlaps() << " overlaps\n";

        // the client view expects the table to be the first element of the first scene
        eventScene()->AddElement(m_table);

        // shapes of the selected overlap are drawn in the 3D view
        auto geoScene = dynamic_cast<REveScene*>(gEve->GetScenes()->FindChild("GeoScene 3D"));
        if (geoScene)
           geoScene->AddElement(m_table->GetVizHolder());
        else
           fwLog(fwlog::kWarning) << "FWWebGeoOverlapTableView no 3D view, overlaps will not be drawn\n";
    }
    catch (const std::runtime_error& e) {
       fwLog(fwlog::kError) << "FWWebGeoOverlapTableView::FWWebGeoOverlapTableView " <<  e.what() << "\n";
    }
    gGeoManager = oldm;
}
