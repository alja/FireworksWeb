// -*- C++ -*-
//
// Package:     Core
// Class  :     FW3DViewGeometry
//
// Implementation:
//     [Notes on implementation]
//
// Original Author:  Alja Mrak-Tadel
//         Created:  Thu Mar 25 22:06:57 CET 2010
//

// system include files
#include <sstream>

// user include files

#include "ROOT/REveManager.hxx"
//#include "ROOT/REveGeoNode.hxx"
#include "ROOT/REveGeoShape.hxx"
#include "ROOT/REveBoxSet.hxx"
#include "ROOT/REveCompound.hxx"
#include "ROOT/REveTrans.hxx"

#include "FireworksWeb/Core/interface/FW3DViewGeometry.h"
#include "FireworksWeb/Core/interface/FWGeometry.h"
//#include "FireworksWeb/Core/interface/TEveElementIter.h"
#include "FireworksWeb/Core/interface/Context.h"
#include "FireworksWeb/Core/interface/FWColorManager.h"
#include "FireworksWeb/Core/interface/fwLog.h"

#include "DataFormats/MuonDetId/interface/DTChamberId.h"
#include "DataFormats/MuonDetId/interface/CSCDetId.h"
#include "DataFormats/MuonDetId/interface/GEMDetId.h"
#include "DataFormats/MuonDetId/interface/ME0DetId.h"

#include "DataFormats/ForwardDetId/interface/MTDDetId.h"

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//

using namespace ROOT::Experimental;

FW3DViewGeometry::FW3DViewGeometry(const fireworks::Context& context)
    : FWViewGeometryList(context, false),
      m_muonBarrelElements(nullptr),
      m_muonBarrelFullElements(nullptr),
      m_muonEndcapElements(nullptr),
      m_muonEndcapFullElements(nullptr),
      m_pixelBarrelElements(nullptr),
      m_pixelEndcapElements(nullptr),
      m_trackerBarrelElements(nullptr),
      m_trackerEndcapElements(nullptr),
      m_HGCalEEElements(nullptr),
      m_HGCalHSiElements(nullptr),
      m_HGCalHScElements(nullptr),
      m_mtdBarrelElements(nullptr),
      m_mtdEndcapElements(nullptr) {
  SetName("3D Geometry");
}

// FW3DViewGeometry::FW3DViewGeometry(const FW3DViewGeometry& rhs)
// {
//    // do actual copying here;
// }

FW3DViewGeometry::~FW3DViewGeometry() {}

//
// member functions
//

//
// const member functions
//

//
// static member functions
//
namespace {
void setCSC(REveCompound* el)
{

  el->CSCApplyMainColorToAllChildren();
  el->CSCApplyMainTransparencyToMatchingChildren();
}
}
void FW3DViewGeometry::showMuonBarrel(bool showMuonBarrel) {
  printf("FW3DViewGeometry::showMuonBarrel\n");
  if (!m_muonBarrelElements && showMuonBarrel) {
    std::cout << "................................................... add moun DT barrel \n";
    m_muonBarrelElements = new REveCompound("DT");
    setCSC(m_muonBarrelElements);


    std::vector<unsigned int> ids;
    for (Int_t iWheel = -2; iWheel <= 2; ++iWheel) {
      for (Int_t iStation = 1; iStation <= 4; ++iStation) {
        // We display only the outer chambers to make the event look more
        // prominent
        if (iWheel == -2 || iWheel == 2 || iStation == 4) {
          for (Int_t iSector = 1; iSector <= 14; ++iSector) {
            if (iStation < 4 && iSector > 12)
              continue;
            DTChamberId id(iWheel, iStation, iSector);
            ids.push_back(id.rawId());
            //REveGeoShape* shape = m_geom->getEveShape(id.rawId());
          }
        }
      }
    }
    REveBoxSet* bs = makeBoxSetFromIds(ids);
    m_muonBarrelElements->AddElement(bs);

    m_muonBarrelElements->SetMainColor(m_context.colorManager()->geomColor(kFWMuonBarrelLineColorIndex));
    AddElement(m_muonBarrelElements);
  }

  if (m_muonBarrelElements) {
    m_muonBarrelElements->SetRnrState(showMuonBarrel);
  }
}

void FW3DViewGeometry::showMuonBarrelFull(bool showMuonBarrel) {
  if (!m_muonBarrelFullElements && showMuonBarrel) {
    m_muonBarrelFullElements = new REveCompound("DT Full");
    setCSC(m_muonBarrelFullElements);
    for (Int_t iWheel = -2; iWheel <= 2; ++iWheel) {
      REveCompound* cWheel = new REveCompound(TString::Format("Wheel %d", iWheel).Data());
      m_muonBarrelFullElements->AddElement(cWheel);
      for (Int_t iStation = 1; iStation <= 4; ++iStation) {
        REveCompound* cStation = new REveCompound(TString::Format("Station %d", iStation).Data());
        cWheel->AddElement(cStation);
        for (Int_t iSector = 1; iSector <= 14; ++iSector) {
          if (iStation < 4 && iSector > 12)
            continue;
          DTChamberId id(iWheel, iStation, iSector);
          REveGeoShape* shape = m_geom->getEveShape(id.rawId());
          shape->SetTitle(TString::Format("DT: W=%d, S=%d, Sec=%d\ndet-id=%u", iWheel, iStation, iSector, id.rawId()).Data());
          addToAunt(shape, kFWMuonBarrelLineColorIndex);
          cStation->AddElement(shape);
        }
      }
    }
    AddElement(m_muonBarrelFullElements);
  }

  if (m_muonBarrelFullElements) {
    m_muonBarrelFullElements->SetRnrState(showMuonBarrel);
    gEve->Redraw3D();
  }
}

//______________________________________________________________________________
void FW3DViewGeometry::showMuonEndcap(bool showMuonEndcap) {
  if (showMuonEndcap && !m_muonEndcapElements) {
    m_muonEndcapElements = new REveCompound("EndCap");
    setCSC(m_muonEndcapElements);
    addToAunt(m_muonEndcapElements, kFWMuonEndcapLineColorIndex);


std::vector<unsigned int> ids;
    for (Int_t iEndcap = 1; iEndcap <= 2; ++iEndcap)  // 1=forward (+Z), 2=backward(-Z)
    {
      /*
      REveCompound* cEndcap = nullptr;
      if (iEndcap == 1)
        cEndcap = new REveCompound("CSC Forward");
      else
        cEndcap = new REveCompound("CSC Backward");
      m_muonEndcapElements->AddElement(cEndcap);
     */

      // Actual CSC geometry:
      // Station 1 has 4 rings with 36 chambers in each
      // Station 2: ring 1 has 18 chambers, ring 2 has 36 chambers
      // Station 3: ring 1 has 18 chambers, ring 2 has 36 chambers
      // Station 4: ring 1 has 18 chambers
      Int_t maxChambers = 36;
      for (Int_t iStation = 1; iStation <= 4; ++iStation) {
        std::ostringstream s;
        s << "Station" << iStation;
        /*
        REveCompound* cStation = new REveCompound(s.str().c_str());
        cEndcap->AddElement(cStation);
        */
        for (Int_t iRing = 1; iRing <= 4; ++iRing) {
          if (iStation > 1 && iRing > 2)
            continue;
          // if( iStation > 3 && iRing > 1 ) continue;
          /*
          std::ostringstream s;
          s << "Ring" << iRing;
          REveCompound* cRing = new REveCompound(s.str().c_str());
          cStation->AddElement(cRing);
          */
          (iRing == 1 && iStation > 1) ? (maxChambers = 18) : (maxChambers = 36);
          for (Int_t iChamber = 1; iChamber <= maxChambers; ++iChamber) {
            Int_t iLayer = 0;  // chamber
            CSCDetId id(iEndcap, iStation, iRing, iChamber, iLayer);

            /*
            REveGeoShape* shape = m_geom->getEveShape(id.rawId());
            shape->SetTitle(TString::Format(
                "CSC: %s, S=%d, R=%d, C=%d\ndet-id=%u", cEndcap->GetCName(), iStation, iRing, iChamber, id.rawId()).Data());

            addToAunt(shape, kFWMuonEndcapLineColorIndex);
            cRing->AddElement(shape);
            */
            ids.push_back(id.rawId());
          }
        }
      }
    }

    // hardcoded gem and me0; need to find better way for different gem geometries
    for (Int_t iRegion = GEMDetId::minRegionId; iRegion <= GEMDetId::maxRegionId; iRegion += 2) {
      /*
      REveCompound* teEndcap = nullptr;
      teEndcap = new REveCompound(Form("GEM Reg=%d", iRegion));
      m_muonEndcapElements->AddElement(teEndcap);
      */
      int iStation = 1;
      {
        /*
        std::ostringstream s;
        s << "Station" << iStation;
        REveCompound* cStation = new REveCompound(s.str().c_str());
        teEndcap->AddElement(cStation);
*/
        for (Int_t iLayer = GEMDetId::minLayerId; iLayer <= GEMDetId::maxLayerId; ++iLayer) {
          int maxChamber = GEMDetId::maxChamberId;

          /*
          std::ostringstream sl;
          sl << "Layer" << iLayer;
          REveCompound* elayer = new REveCompound(sl.str().c_str());
          cStation->AddElement(elayer);
*/
          for (Int_t iChamber = 1; iChamber <= maxChamber; ++iChamber) {
            /*
            std::ostringstream cl;
            cl << "Chamber" << iChamber;
            REveCompound* cha = new REveCompound(cl.str().c_str());
            elayer->AddElement(cha);
*/
            Int_t iRing = 1;
            Int_t iRoll = 0;
            try {
              GEMDetId id(iRegion, iRing, iStation, iLayer, iChamber, iRoll);
              /*
              REveGeoShape* shape = m_geom->getEveShape(id.rawId());
              if (shape) {
                shape->SetTitle(TString::Format(
                    "GEM: , Rng=%d, St=%d, Ch=%d Rl=%d\ndet-id=%u", iRing, iStation, iChamber, iRoll, id.rawId()).Data());

                cha->AddElement(shape);
                addToAunt(shape, kFWMuonEndcapLineColorIndex);
              }*/
              ids.push_back(id.rawId());
            } catch (cms::Exception& e) {
              fwLog(fwlog::kError) << "FW3DViewGeomtery " << e << std::endl;
            }
          }
        }
      }
    }

    // adding me0
    if (m_geom->versionInfo().haveExtraDet("ME0")) {
      for (Int_t iRegion = ME0DetId::minRegionId; iRegion <= ME0DetId::maxRegionId; iRegion = iRegion + 2) {
        /*
        REveCompound* teEndcap = nullptr;
        if (iRegion == 1)
          teEndcap = new REveCompound("ME0 Forward");
        else
          teEndcap = new REveCompound("ME0 Backward");
        m_muonEndcapElements->AddElement(teEndcap);
*/
        for (Int_t iLayer = 1; iLayer <= 6; ++iLayer) {
          /*
          std::ostringstream s;
          s << "Layer" << iLayer;
          REveCompound* cLayer = new REveCompound(s.str().c_str());
          teEndcap->AddElement(cLayer);
*/
          for (Int_t iChamber = 1; iChamber <= 18; ++iChamber) {
            Int_t iRoll = 1;
            // for (Int_t iRoll = ME0DetId::minRollId; iRoll <= ME0DetId::maxRollId ; ++iRoll ){
            ME0DetId id(iRegion, iLayer, iChamber, iRoll);

            /*
            REveGeoShape* shape = m_geom->getEveShape(id.rawId());
            if (shape) {
              shape->SetTitle(TString::Format("ME0: , Ch=%d Rl=%d\ndet-id=%u", iChamber, iRoll, id.rawId()).Data());

              addToAunt(shape, kFWMuonEndcapLineColorIndex);
              cLayer->AddElement(shape);
            }
            */
            ids.push_back(id.rawId());
          }
        }
      }


   
    }

    REveBoxSet* bs = makeBoxSetFromIds(ids);
    m_muonEndcapElements->AddElement(bs);
    AddElement(m_muonEndcapElements);

    m_muonEndcapElements->SetMainColor(m_context.colorManager()->geomColor(kFWMuonEndcapLineColorIndex));
  }

  if (m_muonEndcapElements) {
    m_muonEndcapElements->SetRnrState(showMuonEndcap);
    gEve->Redraw3D();
  }
}

//______________________________________________________________________________
void FW3DViewGeometry::getTransFromId(uint32_t rawId, float* arr )
{
    FWGeometry::IdToInfoItr git = m_geom->find(rawId);
    const FWGeometry::GeomDetInfo& info = *git;


    double array[16] = {info.matrix[0],
                      info.matrix[3],
                      info.matrix[6],
                      0.,
                      info.matrix[1],
                      info.matrix[4],
                      info.matrix[7],
                      0.,
                      info.matrix[2],
                      info.matrix[5],
                      info.matrix[8],
                      0.,
                      info.translation[0],
                      info.translation[1],
                      info.translation[2],
                      1.};

    
    REveTrans trans;
    trans.SetFrom(array);
    trans.SetScale(info.shape[1], info.shape[2], info.shape[3]);

    //float da[16];
    for (int t = 0; t < 16; ++t)
    arr[t] = trans.Array()[t];
}

//______________________________________________________________________________
REveBoxSet* FW3DViewGeometry::makeBoxSetFromIds(std::vector<unsigned int>& ids)
{
    REveBoxSet* bs = new REveBoxSet();
    bs->SetMainColorPtr(new Color_t);
    bs->Reset(REveBoxSet::kBT_InstancedScaledRotated, true, ids.size());
    bs->UseSingleColor();

    float da[16];
    for (std::vector<unsigned int>::const_iterator id = ids.begin(); id != ids.end(); ++id) {
      getTransFromId(*id, da);
      bs->AddInstanceMat4(da);
    }
    bs->SetMainTransparency(90);
    bs->RefitPlex();
    return bs;
}

//______________________________________________________________________________
void FW3DViewGeometry::showPixelBarrel(bool showPixelBarrel) {
  if (showPixelBarrel && !m_pixelBarrelElements) {
    m_pixelBarrelElements = new REveCompound("PixelBarrel");
    setCSC(m_pixelBarrelElements);
    addToAunt(m_pixelBarrelElements, kFWPixelBarrelColorIndex);
    m_pixelBarrelElements->SetRnrState(showPixelBarrel);
    std::vector<unsigned int> ids = m_geom->getMatchedIds(FWGeometry::Tracker, FWGeometry::PixelBarrel);

    REveBoxSet* bs = makeBoxSetFromIds(ids);
    m_pixelBarrelElements->AddElement(bs);
    AddElement(m_pixelBarrelElements);

    m_pixelBarrelElements->SetMainColor(m_context.colorManager()->geomColor(kFWPixelBarrelColorIndex));
  }

  if (m_pixelBarrelElements) {
    m_pixelBarrelElements->SetRnrState(showPixelBarrel);
  }
}

//______________________________________________________________________________
void FW3DViewGeometry::showPixelEndcap(bool showPixelEndcap) {
  if (showPixelEndcap && !m_pixelEndcapElements) {
    m_pixelEndcapElements = new REveCompound("PixelEndcap");
    setCSC(m_pixelEndcapElements);
    addToAunt(m_pixelEndcapElements, kFWPixelEndcapColorIndex);

    std::vector<unsigned int> ids = m_geom->getMatchedIds(FWGeometry::Tracker, FWGeometry::PixelEndcap);
    REveBoxSet* bs = makeBoxSetFromIds(ids);
    m_pixelEndcapElements->AddElement(bs);

    AddElement(m_pixelEndcapElements);
    m_pixelEndcapElements->SetMainColor(m_context.colorManager()->geomColor(kFWPixelEndcapColorIndex));
  }

  if (m_pixelEndcapElements) {
    m_pixelEndcapElements->SetRnrState(showPixelEndcap);
    gEve->Redraw3D();
  }
}

//______________________________________________________________________________
void FW3DViewGeometry::showTrackerBarrel(bool showTrackerBarrel) {
  if (showTrackerBarrel && !m_trackerBarrelElements) {
    m_trackerBarrelElements = new REveCompound("TrackerBarrel");
    setCSC(m_trackerBarrelElements);
    addToAunt(m_trackerBarrelElements, kFWTrackerBarrelColorIndex);

    std::vector<unsigned int> ids = m_geom->getMatchedIds(FWGeometry::Tracker, FWGeometry::TIB);
    REveBoxSet* bs1 = makeBoxSetFromIds(ids);
    m_trackerBarrelElements->AddElement(bs1);
    
    
    ids = m_geom->getMatchedIds(FWGeometry::Tracker, FWGeometry::TOB);
    REveBoxSet* bs2 = makeBoxSetFromIds(ids);
    m_trackerBarrelElements->AddElement(bs2);

    AddElement(m_trackerBarrelElements);
    m_trackerBarrelElements->SetMainColor(m_context.colorManager()->geomColor(kFWTrackerBarrelColorIndex));
  }

  if (m_trackerBarrelElements) {
    m_trackerBarrelElements->SetRnrState(showTrackerBarrel);
  }
}

//______________________________________________________________________________
void FW3DViewGeometry::showTrackerEndcap(bool showTrackerEndcap) {
  if (showTrackerEndcap && !m_trackerEndcapElements) {
    m_trackerEndcapElements = new REveCompound("TrackerEndcap");
    setCSC(m_trackerEndcapElements);
    addToAunt(m_trackerEndcapElements, kFWPixelEndcapColorIndex);

    std::vector<unsigned int> ids = m_geom->getMatchedIds(FWGeometry::Tracker, FWGeometry::TID);
    REveBoxSet* bs1 = makeBoxSetFromIds(ids);
    bs1->SetMainColor(kFWTrackerEndcapColorIndex);
    m_trackerEndcapElements->AddElement(bs1);
    
    ids = m_geom->getMatchedIds(FWGeometry::Tracker, FWGeometry::TEC);
    REveBoxSet* bs2 = makeBoxSetFromIds(ids);
    bs2->SetMainColor(kFWTrackerEndcapColorIndex);
    m_trackerEndcapElements->AddElement(bs2);

    m_trackerEndcapElements->SetMainColor(kFWTrackerEndcapColorIndex);

    AddElement(m_trackerEndcapElements);
  }

  if (m_trackerEndcapElements) {
    m_trackerEndcapElements->SetRnrState(showTrackerEndcap);
  }
}

//______________________________________________________________________________
void FW3DViewGeometry::showHGCalEE(bool showHGCalEE) {
  if (showHGCalEE && !m_HGCalEEElements) {
    m_HGCalEEElements = new REveCompound("HGCalEE");
    auto const ids = m_geom->getMatchedIds(FWGeometry::HGCalEE);
    for (const auto& id : ids) {
      REveGeoShape* shape = m_geom->getHGCSiliconEveShape(id);
      const unsigned int layer = m_geom->getParameters(id)[1];
      const int siIndex = m_geom->getParameters(id)[4];
      shape->SetTitle(Form("HGCalEE %d", layer));
      {
        float color[3] = {0., 0., 0.};
        if (siIndex >= 0 && siIndex < 3)
          color[siIndex] = 1.f;
        shape->SetMainColorRGB(color[0], color[1], color[2]);
        shape->SetPickable(false);
        m_colorComp[kFwHGCalEEColorIndex]->AddNiece(shape);
      }
      m_HGCalEEElements->AddElement(shape);
    }
    AddElement(m_HGCalEEElements);
  }
  if (m_HGCalEEElements) {
    m_HGCalEEElements->SetRnrState(showHGCalEE);
  }
}

void FW3DViewGeometry::showHGCalHSi(bool showHGCalHSi) {
  if (showHGCalHSi && !m_HGCalHSiElements) {
    m_HGCalHSiElements = new REveCompound("HGCalHSi");
    auto const ids = m_geom->getMatchedIds(FWGeometry::HGCalHSi);
    for (const auto& id : ids) {
      REveGeoShape* shape = m_geom->getHGCSiliconEveShape(id);
      const unsigned int layer = m_geom->getParameters(id)[1];
      const int siIndex = m_geom->getParameters(id)[4];
      shape->SetTitle(Form("HGCalHSi %d", layer));
      {
        float color[3] = {0., 0., 0.};
        if (siIndex >= 0 && siIndex < 3)
          color[siIndex] = 1.f;
        shape->SetMainColorRGB(color[0], color[1], color[2]);
        shape->SetPickable(false);
        m_colorComp[kFwHGCalHSiColorIndex]->AddNiece(shape);
      }
      m_HGCalHSiElements->AddElement(shape);
    }
    AddElement(m_HGCalHSiElements);
  }
  if (m_HGCalHSiElements) {
    m_HGCalHSiElements->SetRnrState(showHGCalHSi);
  }
}

void FW3DViewGeometry::showHGCalHSc(bool showHGCalHSc) {
  if (showHGCalHSc && !m_HGCalHScElements) {
    m_HGCalHScElements = new REveCompound("HGCalHSc");
    std::vector<unsigned int> ids = m_geom->getMatchedIds(FWGeometry::HGCalHSc);
    for (const auto& id : m_geom->getMatchedIds(FWGeometry::HGCalHSc)) {
      REveGeoShape* shape = m_geom->getHGCScintillatorEveShape(id);
      const unsigned int layer = m_geom->getParameters(id)[1];
      shape->SetTitle(Form("HGCalHSc %d", layer));
      addToAunt(shape, kFwHGCalHScColorIndex);
      m_HGCalHScElements->AddElement(shape);
    }
    AddElement(m_HGCalHScElements);
  }
  if (m_HGCalHScElements) {
    m_HGCalHScElements->SetRnrState(showHGCalHSc);
  }
}

//______________________________________________________________________________
void FW3DViewGeometry::showMtdBarrel(bool showMtdBarrel) {
  if (showMtdBarrel && !m_mtdBarrelElements) {
    m_mtdBarrelElements = new REveCompound("MtdBarrel");

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
  }

  if (m_mtdBarrelElements) {
    m_mtdBarrelElements->SetRnrState(showMtdBarrel);
  }
}

//______________________________________________________________________________
void FW3DViewGeometry::showMtdEndcap(bool showMtdEndcap) {
  if (showMtdEndcap && !m_mtdEndcapElements) {
    m_mtdEndcapElements = new REveCompound("MtdEndcap");

    std::vector<unsigned int> ids = m_geom->getMatchedIds(FWGeometry::Forward, FWGeometry::PixelBarrel);
    for (std::vector<unsigned int>::const_iterator mtdId = ids.begin(); mtdId != ids.end(); ++mtdId) {
      MTDDetId id(*mtdId);
      if (id.mtdSubDetector() != MTDDetId::MTDType::ETL)
        continue;

      REveGeoShape* shape = m_geom->getEveShape(id.rawId());
      shape->SetTitle(Form("MTD endcap %d", id.rawId()));

      addToAunt(shape, kFWMtdEndcapColorIndex);
      m_mtdEndcapElements->AddElement(shape);
    }
    AddElement(m_mtdEndcapElements);
  }

  if (m_mtdEndcapElements) {
    m_mtdEndcapElements->SetRnrState(showMtdEndcap);
  }
}
