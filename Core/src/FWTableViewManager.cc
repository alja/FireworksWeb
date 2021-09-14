// -*- C++ -*-
//
// Package:     Core
// Class  :     FWTableViewManager
//
// Implementation:
//     <Notes on implementation>
//
// Original Author:
//         Created:  Sun Jan  6 22:01:27 EST 2008
//

// system include files
#include <iostream>
#include <algorithm>
#include <ROOT/REveTableInfo.hxx>
#include <ROOT/REveManager.hxx>
#include <ROOT/REveScene.hxx>

#include "TClass.h"
#include "FWCore/Reflection/interface/BaseWithDict.h"
#include "FWCore/Reflection/interface/MemberWithDict.h"
#include "FWCore/Reflection/interface/FunctionWithDict.h"

// user include files
#include "FireworksWeb/Core/interface/FWTableViewManager.h"
#include "FireworksWeb/Core/interface/FWConfiguration.h"
#include "FireworksWeb/Core/interface/FWEventItem.h"
#include "FireworksWeb/Core/interface/FWTypeToRepresentations.h"
#include "FireworksWeb/Core/interface/fwLog.h"

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//

const std::string FWTableViewManager::kConfigTypeNames = "TypeNames";
const std::string FWTableViewManager::kConfigDisplayedCollection = "DisplayedCollection";


using namespace ROOT::Experimental;

FWTableViewManager::FWTableViewManager():
m_tableInfo(nullptr)
{
   // table specs
   m_tableInfo = new REveTableViewInfo("cmsShowTableInfo");
      
   m_tableInfo->table("reco::Track").
      column("q", 1, "i.charge()").
      column("pt", 1, "i.pt()").
      column("eta", 3, "i.eta()").
      column("phi", 3, "i.phi()").
      column("d0", 5, "i.d0()").
      column("d0Err", 5, "i.d0Error()").
      column("dz", 5, "i.dz()").
      column("dzErr", 5, "i.dzError()").
      column("vx", 5, "i.vx()").
      column("vy", 5, "i.vy()").
      column("vz", 5, "i.vz()").
      column("chi2", 3, "i.chi2()").
      column("pixel hits", 1, "i.hitPattern().numberOfValidPixelHits()").
      column("strip hits", 1, "i.hitPattern().numberOfValidStripHits()").
      column("ndof", 1, "i.ndof()");



  m_tableInfo->table("reco::CaloJet")
      .column("pT", 1, "i.pt()")
      .column("eta", 3, "i.eta()")
      .column("phi", 3, "i.phi()")
      .column("emf", 3, "i.emEnergyFraction()");


   m_tableInfo->table("reco::Muon").
      column("pT", 1, "i.pt()").
      column("eta", 3, "i.eta()").
      column("phi", 3, "i.phi()").
      column("global", 1, "i.isGlobalMuon()").
      column("tracker", 1, "i.isTrackerMuon()").
      column("SA", 1, "i.isStandAloneMuon()").
      column("calo", 1, "i.isCaloMuon()").
      column("tr pt", 1, "i.track().isAvailable() ? i.track()->pt() : -999.000").
      //  column("matches", 1, "i.numberOfMatches('SegmentArbitration')").
      column("d0", 3,    "i.track().isAvailable() ? i.track()->d0() : -999.000").
      column("d0 / d0Err", 3, "i.track().isAvailable()? (i.track()->d0() / i.track()->d0Error()) : -999.000");

   m_tableInfo->table("reco::MET").
      column("et", 1, "i.et()").
      column("phi", 3, "i.phi()").
      column("sumEt", 1, "i.sumEt()").
      column("mEtSig", 3, "i.mEtSig()");

      
   m_tableInfo->table("reco::BeamSpot").
      column("x0", 5, "i.x0()").
      column("y0", 5, "i.y0()").
      column("z0", 5, "i.z0()");

   m_tableInfo->table("reco::GsfElectron").
      column("q", 1, "i.charge()").
      column("pT", 1, "i.pt()").
      column("eta", 3, "i.eta()").
      column("phi", 3, "i.phi()").
      column("E/p", 3, "i.eSuperClusterOverP()").
      column("H/E", 3, "i.hadronicOverEm()").
      column("fbrem", 3, "(i.trackMomentumAtVtx().R() - i.trackMomentumOut().R()) / i.trackMomentumAtVtx().R()").
      column("dei",3, "i.deltaEtaSuperClusterTrackAtVtx()" ).
      column("dpi", 3, "i.deltaPhiSuperClusterTrackAtVtx()");

   m_tableInfo->table("CSCSegment").
      column("endcap", 0, "i.cscDetId().endcap()").
      column("station", 0, "i.cscDetId().station()").
      column("ring", 0, "i.cscDetId().ring()").
      column("chamber", 0, "i.cscDetId().chamber()");

   m_tableInfo->table("reco::Vertex").
      column("x", 5, "i.x()").
      column("xError", 5, "i.xError()").
      column("y", 5, "i.y()").
      column("yError", 5, "i.yError()").
      column("z", 5, "i.z()").
      column("zError", 5, "i.zError()").
      column("tracks", 1, "i.tracksSize()").
      column("chi2", 3, "i.chi2()").
      column("ndof", 3, "i.ndof()");

   
   m_tableInfo->table("pat::PackedCandidate").
      column("pT", 1, "i.pt()").
      column("eta", 3, "i.eta()").
      column("phi", 3, "i.phi()").
      column("pdgId", 0, "i.pdgId()").
      column("charge", 0, "i.charge()").
      column("dxy", 3, "i.dxy()").
      column("dzAssociatedPV", 3, "i.dzAssociatedPV()");

   m_tableInfo->table("reco::PFCandidate").
      column("et", 1, "i.et()").
      column("eta", 3, "i.eta()").
      column("phi", 3, "i.phi()").
      column("ecalEnergy", 3,"i.ecalEnergy()").
      column("hcalEnergy", 3,"i.hcalEnergy()").
      column("track pt", 3,"i.trackRef().isAvailable() ? i.trackRef()->pt() : -999.000");
   
   m_tableInfo->table("reco::Candidate").
      column("pt", 1, "i.pt()").
      column("eta", 3, "i.eta()");

   m_tableInfo->table("reco::LeafCandidate").column("leafCpt", 1, "i.pt()").column("leafCeta", 3, "i.eta()");
}

const std::string& FWTableViewManager::getDisplayedCollection() const
{
   // std::cout << m_defaultDisplayedCollection << std::endl;
   return m_defaultDisplayedCollection;
}

void 
FWTableViewManager::addTo (FWConfiguration &iTo) const
{
   FWConfiguration dcol(7);
   dcol.addValue(getDisplayedCollection());
   iTo.addKeyValue(kConfigDisplayedCollection, dcol);

   FWConfiguration typeNames(7);
   char prec[100];
   for (auto it = m_tableInfo->RefSpecs().begin(); it != m_tableInfo->RefSpecs().end(); ++it)
   {
      const std::string& typeName = it->first;
      FWConfiguration columns(1);
      REveTableHandle::Entries_t &entries = it->second;
      for ( auto& entry : entries)
      {
         columns.addValue(entry.fName);
         columns.addValue(entry.fExpression);
         columns.addValue((snprintf(prec, 100, "%d", entry.fPrecision), prec));
      }
      typeNames.addKeyValue(typeName, columns);
   }
   iTo.addKeyValue(kConfigTypeNames, typeNames);
}

void 
FWTableViewManager::setFrom(const FWConfiguration &iFrom)
{
   try
   {
      std::string orig = m_defaultDisplayedCollection;
      m_defaultDisplayedCollection = iFrom.valueForKey(kConfigDisplayedCollection)->value();
      if (orig != m_defaultDisplayedCollection)
      {
         auto s = gEve->GetScenes()->FindChild("Collections");

         auto ce = s->FindChild(m_defaultDisplayedCollection);
         if (ce)
         {
            m_tableInfo->SetDisplayedCollection(ce->GetElementId());
         }
         else
         {
            // if configuration is dumped programatically, the Table configuration is last threfore after collection definition
            std::cout << "FWTableViewManager::setFrom() can't locate collection " << m_defaultDisplayedCollection << ", num collections "<< s->NumChildren() << std::endl;
         }
      }

      const FWConfiguration *typeNames = iFrom.valueForKey(kConfigTypeNames);
      if (typeNames == nullptr)
      {
         fwLog(fwlog::kWarning) << "no table column configuration stored, using defaults\n";
         return;
      }

      const FWConfiguration::KeyValues *keyValues = typeNames->keyValues();

      for (FWConfiguration::KeyValues::const_iterator iType = keyValues->begin(); iType != keyValues->end(); ++iType)
      {
         const FWConfiguration *columns = &iType->second;
         assert(columns != nullptr);
         for (FWConfiguration::StringValuesIt
                  it = columns->stringValues()->begin(),
                  itEnd = columns->stringValues()->end();
              it != itEnd; ++it)
         {
            const std::string &name = *it++;
            const std::string &expr = *it++;
            int prec = atoi(it->c_str());
            m_tableInfo->table(iType->first).column(name.c_str(), prec, expr.c_str());
         }
      }
   }
   catch (...)
   {
      // No info about types in the configuration; this is not an
      // error, it merely means that the types are handled by the
      // first FWTableView.
   }
}

void 
FWTableViewManager::checkExpressionsForType(TClass* itemclass)
{
   //printf("checkExpressionsForType");
}
