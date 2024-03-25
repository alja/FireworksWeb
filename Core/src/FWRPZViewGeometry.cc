// -*- C++ -*-
//
// Package:     Core
// Class  :     FWRPZViewGeometry
//
// Implementation:
//     [Notes on implementation]
//
// Original Author:  Alja Mrak-Tadel
//         Created:  Thu Mar 25 20:33:06 CET 2010
//

// system include files
#include <iostream>
#include <cassert>

// user include files
#include "TGeoBBox.h"

#include "ROOT/REveProjectionManager.hxx"
#include "ROOT/REveStraightLineSet.hxx"
#include "ROOT/REvePointSet.hxx"
#include "ROOT/REveGeoShape.hxx"
#include "ROOT/REveCompound.hxx"

#include "FireworksWeb/Core/interface/FWRPZViewGeometry.h"
#include "FireworksWeb/Core/interface/FWGeometry.h"
#include "FireworksWeb/Core/interface/Context.h"
#include "FireworksWeb/Core/interface/fwLog.h"

#include "DataFormats/MuonDetId/interface/DTChamberId.h"
#include "DataFormats/MuonDetId/interface/CSCDetId.h"
#include "DataFormats/MuonDetId/interface/RPCDetId.h"
#include "DataFormats/MuonDetId/interface/GEMDetId.h"
#include "DataFormats/MuonDetId/interface/ME0DetId.h"

#include "DataFormats/ForwardDetId/interface/MTDDetId.h"

using namespace ROOT::Experimental;


namespace 
{
  void setRnrStateRec(bool x, REveElement* e)
  {
    e->SetRnrState(x);
    for (auto c : e->RefChildren())
    {
      setRnrStateRec(x, c);
    }
  }
}

FWRPZViewGeometry::FWRPZViewGeometry(const fireworks::Context& context)
    : FWViewGeometryList(context, true),

      m_rhoPhiGeo(nullptr),
      m_rhoZGeo(nullptr),

      m_pixelBarrelElements(nullptr),
      m_pixelEndcapElements(nullptr),
      m_trackerBarrelElements(nullptr),
      m_trackerEndcapElements(nullptr),
      m_rpcEndcapElements(nullptr),
      m_GEMElements(nullptr),
      m_ME0Elements(nullptr) {
      SetName("RPZGeomShared");
      }

// FWRPZViewGeometry::FWRPZViewGeometry(const FWRPZViewGeometry& rhs)
// {
//    // do actual copying here;
// }

FWRPZViewGeometry::~FWRPZViewGeometry() {
  m_rhoPhiGeo->DecDenyDestroy();
  m_rhoZGeo->DecDenyDestroy();
}

//______________________________________________________________________________

REveElement* FWRPZViewGeometry::initStdGeoElements(const std::string& type)
{
  if (m_geom->isEmpty())
    return nullptr;

  REveElement* sg = new REveElement("stdGeo");
  this->AddElement(sg);

  if (type == "RhoZ") {
    sg->AddElement(makeMuonGeometryRhoZ());
    sg->AddElement(makeCaloOutlineRhoZ());
  } else {
   sg->AddElement(makeMuonGeometryRhoPhi());
   sg->AddElement(makeCaloOutlineRhoPhi());
  }
  return sg;
}

//______________________________________________________________________________

REveElement* FWRPZViewGeometry::makeCaloOutlineRhoZ() {
  using namespace fireworks;

  float ri = m_context.caloZ2() * tan(2 * atan(exp(-m_context.caloMaxEta())));

  REveStraightLineSet* el = new REveStraightLineSet("TrackerRhoZoutline");
  el->SetPickable(kFALSE);
  addToAunt(el, kFWTrackerBarrelColorIndex, false);

  el->AddLine(0, m_context.caloR1(), -m_context.caloZ1(), 0, m_context.caloR1(), m_context.caloZ1());
  el->AddLine(0, -m_context.caloR1(), m_context.caloZ1(), 0, -m_context.caloR1(), -m_context.caloZ1());

  el->AddLine(0, -m_context.caloR2(), m_context.caloZ2(), 0, -ri, m_context.caloZ2());
  el->AddLine(0, ri, m_context.caloZ2(), 0, m_context.caloR2(), m_context.caloZ2());

  el->AddLine(0, -m_context.caloR2(), -m_context.caloZ2(), 0, -ri, -m_context.caloZ2());
  el->AddLine(0, ri, -m_context.caloZ2(), 0, m_context.caloR2(), -m_context.caloZ2());

  return el;
}

REveElement* FWRPZViewGeometry::makeCaloOutlineRhoPhi() {
  REveStraightLineSet* el = new REveStraightLineSet("TrackerRhoPhi");
  addToAunt(el, kFWTrackerBarrelColorIndex, false);
  el->SetPickable(false);

  el->SetLineColor(m_context.colorManager()->geomColor(kFWTrackerBarrelColorIndex));
  //el->SetLineColor(kFWTrackerBarrelColorIndex);
  const unsigned int nSegments = 100;
  const double r = m_context.caloR1();
  for (unsigned int i = 1; i <= nSegments; ++i)
    el->AddLine(r * sin(TMath::TwoPi() / nSegments * (i - 1)),
                r * cos(TMath::TwoPi() / nSegments * (i - 1)),
                0,
                r * sin(TMath::TwoPi() / nSegments * i),
                r * cos(TMath::TwoPi() / nSegments * i),
                0);

  REvePointSet* ref = new REvePointSet("reference");
  ref->SetTitle("(0,0,0)");
  ref->SetMarkerStyle(4);
  ref->SetMarkerColor(kWhite);
  ref->SetNextPoint(0., 0., 0.);
  el->AddElement(ref);

  return el;
}

//______________________________________________________________________________

REveElement* FWRPZViewGeometry::makeMuonGeometryRhoPhi(void) {
  Int_t iWheel = 0;

  // rho-phi view
  REveCompound* container = new REveCompound("MuonRhoPhi");

  for (Int_t iStation = 1; iStation <= 4; ++iStation) {
    for (Int_t iSector = 1; iSector <= 14; ++iSector) {
      if (iStation < 4 && iSector > 12)
        continue;
      DTChamberId id(iWheel, iStation, iSector);
      REveGeoShape* shape = m_geom->getEveShape(id.rawId());
      if (shape) {
        shape->SetMainColor(kFWMuonBarrelLineColorIndex);
        shape->SetMainTransparency(70);
        addToAunt(shape, kFWMuonBarrelLineColorIndex);
        container->AddElement(shape);
        shape->SetLineColor(kMagenta);
      }
    }
  }
  return container;
}
namespace {

  //void addLibe

}
//______________________________________________________________________________

REveElement* FWRPZViewGeometry::makeMuonGeometryRhoZ(void) {
  REveElement* container = new REveElement("MuonRhoZ");

  {
    REveCompound* dtContainer = new REveCompound("DT");
    //dtContainer->SetMainTransparency(80);
    //dtContainer->CSCApplyMainTransparencyToAllChildren();
    for (Int_t iWheel = -2; iWheel <= 2; ++iWheel) {
      for (Int_t iStation = 1; iStation <= 4; ++iStation) {
        float min_rho(1000), max_rho(0), min_z(2000), max_z(-2000);

        // This will give us a quarter of DTs
        // which is enough for our projection
        for (Int_t iSector = 1; iSector <= 4; ++iSector) {
          DTChamberId id(iWheel, iStation, iSector);
          unsigned int rawid = id.rawId();
          FWGeometry::IdToInfoItr det = m_geom->find(rawid);
          if (det == m_geom->mapEnd())
            return container;
          estimateProjectionSizeDT(*det, min_rho, max_rho, min_z, max_z);
        }
        if (min_rho > max_rho || min_z > max_z)
          continue;
        REveElement* se = makeShape(min_rho, max_rho, min_z, max_z);
        se->SetMainTransparency(50);
        addToAunt(se, kFWMuonBarrelLineColorIndex);
        dtContainer->AddElement(se);
        se = makeShape(-max_rho, -min_rho, min_z, max_z);
        se->SetMainTransparency(50);
        addToAunt(se, kFWMuonBarrelLineColorIndex);
        dtContainer->AddElement(se);
      }
    }

    container->AddElement(dtContainer);
  }
  {
    // addcsc
    REveCompound* cscContainer = new REveCompound("CSC");
    cscContainer->SetMainTransparency(80);
    std::vector<CSCDetId> ids;
    for (int endcap = CSCDetId::minEndcapId(); endcap <= CSCDetId::maxEndcapId(); ++endcap) {
      for (int station = 1; station <= 4; ++station) {
        ids.push_back(CSCDetId(endcap, station, 2, 10, 0));  //outer ring up
        ids.push_back(CSCDetId(endcap, station, 2, 11, 0));  //outer ring up

        ids.push_back(CSCDetId(endcap, station, 2, 28, 0));  //outer ring down
        ids.push_back(CSCDetId(endcap, station, 2, 29, 0));  //outer ring down

        ids.push_back(CSCDetId(endcap, station, 1, 5, 0));  //inner ring up
        ids.push_back(CSCDetId(endcap, station, 1, 6, 0));  //inner ring up

        int off = (station == 1) ? 10 : 0;
        ids.push_back(CSCDetId(endcap, station, 1, 15 + off, 0));  //inner ring down
        ids.push_back(CSCDetId(endcap, station, 1, 16 + off, 0));  //inner ring down
      }
      ids.push_back(CSCDetId(endcap, 1, 3, 10, 0));  // ring 3 down
      ids.push_back(CSCDetId(endcap, 1, 3, 28, 0));  // ring 3 down
    }
    for (std::vector<CSCDetId>::iterator i = ids.begin(); i != ids.end(); ++i) {
      unsigned int rawid = i->rawId();
      REveGeoShape* shape = m_geom->getEveShape(rawid);
      shape->SetMainTransparency(50);
      if (!shape)
        return cscContainer;
      addToAunt(shape, kFWMuonEndcapLineColorIndex);
      shape->SetName(Form(" e:%d r:%d s:%d chamber %d", i->endcap(), i->ring(), i->station(), i->chamber()));
      cscContainer->AddElement(shape);
    }
    container->AddElement(cscContainer);
    container->SetMainTransparency(50);
  }

  return container;
}

//______________________________________________________________________________

REveGeoShape* FWRPZViewGeometry::makeShape(double min_rho, double max_rho, double min_z, double max_z) {
  REveTrans t;
  t(1, 1) = 1;
  t(1, 2) = 0;
  t(1, 3) = 0;
  t(2, 1) = 0;
  t(2, 2) = 1;
  t(2, 3) = 0;
  t(3, 1) = 0;
  t(3, 2) = 0;
  t(3, 3) = 1;
  t(1, 4) = 0;
  t(2, 4) = (min_rho + max_rho) / 2;
  t(3, 4) = (min_z + max_z) / 2;

  REveGeoShape* shape = new REveGeoShape;
  shape->SetTransMatrix(t.Array());

  shape->SetRnrSelf(kTRUE);
  shape->SetRnrChildren(kTRUE);
  TGeoBBox* box = new TGeoBBox(0, (max_rho - min_rho) / 2, (max_z - min_z) / 2);
  shape->SetShape(box);

  return shape;
}

//______________________________________________________________________________

void FWRPZViewGeometry::estimateProjectionSizeDT(
    const FWGeometry::GeomDetInfo& info, float& min_rho, float& max_rho, float& min_z, float& max_z) {
  // we will test 5 points on both sides ( +/- z)
  float local[3], global[3];

  float dX = info.shape[1];
  float dY = info.shape[2];
  float dZ = info.shape[3];

  local[0] = 0;
  local[1] = 0;
  local[2] = dZ;
  m_geom->localToGlobal(info, local, global);
  estimateProjectionSize(global, min_rho, max_rho, min_z, max_z);

  local[0] = dX;
  local[1] = dY;
  local[2] = dZ;
  m_geom->localToGlobal(info, local, global);
  estimateProjectionSize(global, min_rho, max_rho, min_z, max_z);

  local[0] = -dX;
  local[1] = dY;
  local[2] = dZ;
  m_geom->localToGlobal(info, local, global);
  estimateProjectionSize(global, min_rho, max_rho, min_z, max_z);

  local[0] = dX;
  local[1] = -dY;
  local[2] = dZ;
  m_geom->localToGlobal(info, local, global);
  estimateProjectionSize(global, min_rho, max_rho, min_z, max_z);

  local[0] = -dX;
  local[1] = -dY;
  local[2] = dZ;
  m_geom->localToGlobal(info, local, global);
  estimateProjectionSize(global, min_rho, max_rho, min_z, max_z);

  local[0] = 0;
  local[1] = 0;
  local[2] = -dZ;
  m_geom->localToGlobal(info, local, global);
  estimateProjectionSize(global, min_rho, max_rho, min_z, max_z);

  local[0] = dX;
  local[1] = dY;
  local[2] = -dZ;
  m_geom->localToGlobal(info, local, global);
  estimateProjectionSize(global, min_rho, max_rho, min_z, max_z);

  local[0] = -dX;
  local[1] = dY;
  local[2] = -dZ;
  m_geom->localToGlobal(info, local, global);
  estimateProjectionSize(global, min_rho, max_rho, min_z, max_z);

  local[0] = dX;
  local[1] = -dY;
  local[2] = -dZ;
  m_geom->localToGlobal(info, local, global);
  estimateProjectionSize(global, min_rho, max_rho, min_z, max_z);

  local[0] = -dX;
  local[1] = -dY;
  local[2] = -dZ;
  m_geom->localToGlobal(info, local, global);
  estimateProjectionSize(global, min_rho, max_rho, min_z, max_z);
}

void FWRPZViewGeometry::estimateProjectionSize(
    const float* global, float& min_rho, float& max_rho, float& min_z, float& max_z) {
  double rho = sqrt(global[0] * global[0] + global[1] * global[1]);
  if (min_rho > rho)
    min_rho = rho;
  if (max_rho < rho)
    max_rho = rho;
  if (min_z > global[2])
    min_z = global[2];
  if (max_z < global[2])
    max_z = global[2];
}

// ATODO:: check white vertex -> shouldn't be relative to background
//         when detruction ?

// ATODO why color is not set in 3D original, why cast to polygonsetprojected after projected ????
// is geom color dynamic --- independent of projection manager

// NOTE geomtry MuonRhoZAdanced renamed to  MuonRhoZ

//==============================================================================
//==============================================================================

void FWRPZViewGeometry::showPixelBarrel(bool show) {
  if (!m_pixelBarrelElements && show) {
    m_pixelBarrelElements = new REveElement("PixelBarrel");
    AddElement(m_pixelBarrelElements);
    std::vector<unsigned int> ids = m_geom->getMatchedIds(FWGeometry::Tracker, FWGeometry::PixelBarrel);
    for (std::vector<unsigned int>::const_iterator id = ids.begin(); id != ids.end(); ++id) {
      REveGeoShape* shape = m_geom->getEveShape(*id);
      if (!shape)
        return;
      shape->SetTitle(Form("PixelBarrel %d", *id));
      addToAunt(shape, kFWPixelBarrelColorIndex);
      m_pixelBarrelElements->AddElement(shape);
    }
    importNew(m_pixelBarrelElements);
  }

  if (m_pixelBarrelElements) {
    setRnrStateRec(show, m_pixelBarrelElements);
  }
}

void FWRPZViewGeometry::showPixelEndcap(bool show) {
  if (!m_pixelEndcapElements && show) {
    m_pixelEndcapElements = new REveElement("PixelEndcap");

    std::vector<unsigned int> ids = m_geom->getMatchedIds(FWGeometry::Tracker, FWGeometry::PixelEndcap);
    for (std::vector<unsigned int>::const_iterator id = ids.begin(); id != ids.end(); ++id) {
      REveGeoShape* shape = m_geom->getEveShape(*id);
      if (!shape)
        return;
      shape->SetTitle(Form("PixelEndCap %d", *id));
      addToAunt(shape, kFWPixelEndcapColorIndex);
      m_pixelEndcapElements->AddElement(shape);
    }

    AddElement(m_pixelEndcapElements);
    importNew(m_pixelEndcapElements);
  }

  if (m_pixelEndcapElements) {
    //m_pixelEndcapElements->setRnrStateRec(show);
    setRnrStateRec(show, m_pixelEndcapElements);
    // gEve->Redraw3D();
  }
}

void FWRPZViewGeometry::showTrackerBarrel(bool show) {
  if (!m_trackerBarrelElements && show) {
    m_trackerBarrelElements = new REveElement("TrackerBarrel");

    std::vector<unsigned int> ids = m_geom->getMatchedIds(FWGeometry::Tracker, FWGeometry::TIB);
    for (std::vector<unsigned int>::const_iterator id = ids.begin(); id != ids.end(); ++id) {
      REveGeoShape* shape = m_geom->getEveShape(*id);
      if (!shape)
        return;
      addToAunt(shape, kFWTrackerBarrelColorIndex);
      m_trackerBarrelElements->AddElement(shape);
    }
    ids = m_geom->getMatchedIds(FWGeometry::Tracker, FWGeometry::TOB);
    for (std::vector<unsigned int>::const_iterator id = ids.begin(); id != ids.end(); ++id) {
      REveGeoShape* shape = m_geom->getEveShape(*id);
      if (!shape)
        return;
      shape->SetTitle(Form("TrackerBarrel %d", *id));
      addToAunt(shape, kFWTrackerBarrelColorIndex);
      m_trackerBarrelElements->AddElement(shape);
    }

    AddElement(m_trackerBarrelElements);
    importNew(m_trackerBarrelElements);
  }

  if (m_trackerBarrelElements) {
    setRnrStateRec(show, m_trackerBarrelElements);
    // gEve->Redraw3D();
  }
}

void FWRPZViewGeometry::showTrackerEndcap(bool show) {
  if (!m_trackerEndcapElements && show) {
    m_trackerEndcapElements = new REveElement("TrackerEndcap");

    std::vector<unsigned int> ids = m_geom->getMatchedIds(FWGeometry::Tracker, FWGeometry::TID);
    for (std::vector<unsigned int>::const_iterator id = ids.begin(); id != ids.end(); ++id) {
      REveGeoShape* shape = m_geom->getEveShape(*id);
      addToAunt(shape, kFWTrackerEndcapColorIndex);

      if (!shape)
        return;
      m_trackerEndcapElements->AddElement(shape);
    }
    ids = m_geom->getMatchedIds(FWGeometry::Tracker, FWGeometry::TEC);
    for (std::vector<unsigned int>::const_iterator id = ids.begin(); id != ids.end(); ++id) {
      REveGeoShape* shape = m_geom->getEveShape(*id);
      shape->SetTitle(Form("TrackerEndcap %d", *id));
      if (!shape)
        return;
      addToAunt(shape, kFWTrackerEndcapColorIndex);
      m_trackerEndcapElements->AddElement(shape);
    }

    AddElement(m_trackerEndcapElements);
    importNew(m_trackerEndcapElements);
  }

  if (m_trackerEndcapElements) {
   setRnrStateRec(show,  m_trackerEndcapElements);
    // gEve->Redraw3D();
  }
}

//---------------------------------------------------------
void FWRPZViewGeometry::showRpcEndcap(bool show) {
  if (!m_rpcEndcapElements && show) {
    m_rpcEndcapElements = new REveElement("RpcEndcap");

    std::vector<RPCDetId> ids;
    int mxSt = m_geom->versionInfo().haveExtraDet("RE4") ? 4 : 3;
    for (int region = -1; region <= 1; ++region) {
      if (region == 0)
        continue;
      for (int ring = 2; ring <= 3; ++ring) {
        for (int station = 1; station <= mxSt; ++station) {
          int sector = 1;
          ids.push_back(RPCDetId(region, ring, station, sector, 1, 1, 1));
          ids.push_back(RPCDetId(region, ring, station, sector, 1, 1, 2));
          ids.push_back(RPCDetId(region, ring, station, sector, 1, 1, 3));
          if (ring == 2 && station == 1) {  // 2 layers in ring 2 station 1 up
            ids.push_back(RPCDetId(region, ring, station, sector, 1, 2, 1));
            ids.push_back(RPCDetId(region, ring, station, sector, 1, 2, 2));
            ids.push_back(RPCDetId(region, ring, station, sector, 1, 2, 3));
          }
          sector = 5;
          ids.push_back(RPCDetId(region, ring, station, sector, 1, 1, 1));
          ids.push_back(RPCDetId(region, ring, station, sector, 1, 1, 2));
          ids.push_back(RPCDetId(region, ring, station, sector, 1, 1, 3));

          if (ring == 2 && station == 1) {  // 2 layers in ring 2 station 1 down
            ids.push_back(RPCDetId(region, ring, station, sector, 1, 2, 1));
            ids.push_back(RPCDetId(region, ring, station, sector, 1, 2, 2));
            ids.push_back(RPCDetId(region, ring, station, sector, 1, 2, 3));
          }
        }
      }
    }

    for (std::vector<RPCDetId>::iterator i = ids.begin(); i != ids.end(); ++i) {
      REveGeoShape* shape = m_geom->getEveShape(i->rawId());
      if (!shape)
        return;
      addToAunt(shape, kFWMuonEndcapLineColorIndex);
      m_rpcEndcapElements->AddElement(shape);
      // gEve->AddToListTree(shape, true);
    }

    AddElement(m_rpcEndcapElements);
    importNew(m_rpcEndcapElements);
  }

  if (m_rpcEndcapElements) {
    setRnrStateRec(show,m_rpcEndcapElements);
    // gEve->Redraw3D();
  }
}

//______________________________________________________________________________

void FWRPZViewGeometry::showGEM(bool show) {
  // hardcoded gem and me0; need to find better way for different gem geometries
  if (!m_GEMElements && show) {
    m_GEMElements = new REveElement("GEM");

    for (Int_t iRegion = GEMDetId::minRegionId; iRegion <= GEMDetId::maxRegionId; iRegion = iRegion + 2) {
      int iStation = 1;
      int iRing = 1;
      int iLayer = 1;

      int carr[4] = {10, 11, 29, 30};
      for (int i = 0; i < 4; ++i) {
        int iChamber = carr[i];
        int iRoll = 0;
        try {
          GEMDetId id(iRegion, iRing, iStation, iLayer, iChamber, iRoll);
          REveGeoShape* shape = m_geom->getEveShape(id.rawId());
          if (shape) {
            addToAunt(shape, kFWMuonEndcapLineColorIndex);
            m_GEMElements->AddElement(shape);
            // gEve->AddToListTree(shape, true);
          }
        } catch (cms::Exception& e) {
          fwLog(fwlog::kError) << "FWRPZViewGeomtery " << e << std::endl;
        }
      }
    }

    AddElement(m_GEMElements);
    importNew(m_GEMElements);
  }

  if (m_GEMElements) {
    setRnrStateRec(show,m_GEMElements);
    // gEve->Redraw3D();
  }
}

void FWRPZViewGeometry::showME0(bool show) {
  if (!m_ME0Elements && show) {
    m_ME0Elements = new REveElement("ME0");

    for (Int_t iRegion = ME0DetId::minRegionId; iRegion <= ME0DetId::maxRegionId; iRegion = iRegion + 2) {
      for (Int_t iLayer = 1; iLayer <= 6; ++iLayer) {
        for (Int_t iChamber = 1; iChamber <= 18; ++iChamber) {
          Int_t iRoll = 1;
          ME0DetId id(iRegion, iLayer, iChamber, iRoll);
          REveGeoShape* shape = m_geom->getEveShape(id.rawId());
          if (shape) {
            addToAunt(shape, kFWMuonEndcapLineColorIndex);
            m_ME0Elements->AddElement(shape);
            // gEve->AddToListTree(shape, true);
          }
        }
      }
    }

    AddElement(m_ME0Elements);
    importNew(m_ME0Elements);
  }
  if (m_ME0Elements) {
   setRnrStateRec(show,  m_ME0Elements);
    // gEve->Redraw3D();
  }
}

//______________________________________________________________________________

void FWRPZViewGeometry::showMtdBarrel(bool show) {
  if (!m_mtdBarrelElements && show) {
    m_mtdBarrelElements = new REveElement("MtdBarrel");

    std::vector<unsigned int> ids = m_geom->getMatchedIds(FWGeometry::Forward, FWGeometry::PixelBarrel);
    for (std::vector<unsigned int>::const_iterator mtdId = ids.begin(); mtdId != ids.end(); ++mtdId) {
      MTDDetId id(*mtdId);
      if (id.mtdSubDetector() != MTDDetId::MTDType::BTL)
        continue;

      REveGeoShape* shape = m_geom->getEveShape(id.rawId());
      shape->SetTitle(Form("MTD barrel %d", id.rawId()));

      addToAunt(shape, kFWMtdBarrelColorIndex);
      m_mtdBarrelElements->AddElement(shape);
    }

    AddElement(m_mtdBarrelElements);
    importNew(m_mtdBarrelElements);
  }

  if (m_mtdBarrelElements) {
    setRnrStateRec(show, m_mtdBarrelElements);
    // gEve->Redraw3D();
  }
}

void FWRPZViewGeometry::showMtdEndcap(bool show) {
  if (!m_mtdEndcapElements && show) {
    m_mtdEndcapElements = new REveElement("MtdEndcap");

    REveElement* disk1ZposUp =
        makeShape(m_context.mtdEtlR1(), m_context.mtdEtlR2(), m_context.mtdEtlZ1(1), m_context.mtdEtlZ2(1));
    addToAunt(disk1ZposUp, kFWMtdEndcapColorIndex);
    m_mtdEndcapElements->AddElement(disk1ZposUp);
    REveElement* disk1ZposDw =
        makeShape(-m_context.mtdEtlR1(), -m_context.mtdEtlR2(), m_context.mtdEtlZ1(1), m_context.mtdEtlZ2(1));
    addToAunt(disk1ZposDw, kFWMtdEndcapColorIndex);
    m_mtdEndcapElements->AddElement(disk1ZposDw);

    REveElement* disk2ZposUp =
        makeShape(m_context.mtdEtlR1(), m_context.mtdEtlR2(), m_context.mtdEtlZ1(2), m_context.mtdEtlZ2(2));
    addToAunt(disk2ZposUp, kFWMtdEndcapColorIndex);
    m_mtdEndcapElements->AddElement(disk2ZposUp);
    REveElement* disk2ZposDw =
        makeShape(-m_context.mtdEtlR1(), -m_context.mtdEtlR2(), m_context.mtdEtlZ1(2), m_context.mtdEtlZ2(2));
    addToAunt(disk2ZposDw, kFWMtdEndcapColorIndex);
    m_mtdEndcapElements->AddElement(disk2ZposDw);

    REveElement* disk1ZnegUp =
        makeShape(m_context.mtdEtlR1(), m_context.mtdEtlR2(), -m_context.mtdEtlZ1(1), -m_context.mtdEtlZ2(1));
    addToAunt(disk1ZnegUp, kFWMtdEndcapColorIndex);
    m_mtdEndcapElements->AddElement(disk1ZnegUp);
    REveElement* disk1ZnegDw =
        makeShape(-m_context.mtdEtlR1(), -m_context.mtdEtlR2(), -m_context.mtdEtlZ1(1), -m_context.mtdEtlZ2(1));
    addToAunt(disk1ZnegDw, kFWMtdEndcapColorIndex);
    m_mtdEndcapElements->AddElement(disk1ZnegDw);

    REveElement* disk2ZnegUp =
        makeShape(m_context.mtdEtlR1(), m_context.mtdEtlR2(), -m_context.mtdEtlZ1(2), -m_context.mtdEtlZ2(2));
    addToAunt(disk2ZnegUp, kFWMtdEndcapColorIndex);
    m_mtdEndcapElements->AddElement(disk2ZnegUp);
    REveElement* disk2ZnegDw =
        makeShape(-m_context.mtdEtlR1(), -m_context.mtdEtlR2(), -m_context.mtdEtlZ1(2), -m_context.mtdEtlZ2(2));
    addToAunt(disk2ZnegDw, kFWMtdEndcapColorIndex);
    m_mtdEndcapElements->AddElement(disk2ZnegDw);

    AddElement(m_mtdEndcapElements);
    importNew(m_mtdEndcapElements);
  }

  if (m_mtdEndcapElements) {
    setRnrStateRec(show, m_mtdEndcapElements);
    // gEve->Redraw3D();
  }
}

//-------------------------------------

void FWRPZViewGeometry::importNew(REveElement* x) {
  REveProjected* proj = *RefProjecteds().begin();
  proj->GetManager()->SubImportElements(x, proj->GetProjectedAsElement());
}
