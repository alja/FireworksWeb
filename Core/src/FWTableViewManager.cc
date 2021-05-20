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


   m_tableInfo->table("reco::CaloJet").
      column("pT", 1, "i.pt()").
      column("eta", 3, "i.eta()").
      column("phi", 3, "i.phi()").
      column("ECAL", 1, "i.p4().E() * i.emEnergyFraction()").
      column("HCAL", 1, "i.p4().E() * i.energyFractionHadronic()").
      column("emf", 3, "i.emEnergyFraction()");

   m_tableInfo->table("reco::Muon").
      column("pT", 1, "i.pt()").
      column("eta", 3, "i.eta()").
      column("phi", 3, "i.phi()").
      column("global", 1, "i.isGlobalMuon()").
      column("tracker", 1, "i.isTrackerMuon()").
      column("SA", 1, "i.isStandAloneMuon()").
      column("calo", 1, "i.isCaloMuon()").
      column("tr pt", 1, "i.track()->pt()").
      //  column("matches", 1, "i.numberOfMatches('SegmentArbitration')").
      column("d0", 3, "i.track()->d0()").
      column("d0 / d0Err", 3, "i.track()->d0() / i.track()->d0Error()");
    
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
      column("et", 1, "i.Et()").
      column("eta", 3, "i.eta()").
      column("phi", 3, "i.phi()").
      column("ecalEnergy", 3,".ecalEnergy()").
      column("hcalEnergy", 3,"i.hcalEnergy()").
      column("track pt", 3,"i.trackRef().pt()");
   
   m_tableInfo->table("reco::Candidate").
      column("pt", 1, "i.pt()").
      column("eta", 3, "i.eta()");

   m_tableInfo->table("reco::LeafCandidate").
      column("leafCpt", 1, "i.pt()").
      column("leafCeta", 3, "i.eta()");

   m_displayedCollection= "MET";
}

void 
FWTableViewManager::addTo (FWConfiguration &iTo) const
{
   /*
   FWConfiguration typeNames(1);
   char prec[100];

   for (TableSpecs::const_iterator 
	iType = m_tableFormats.begin(),
	iType_end = m_tableFormats.end();
	iType != iType_end; ++iType) 
   {
      const std::string &typeName = iType->first;
      typeNames.addValue(typeName);
      FWConfiguration columns(1);
      const TableEntries &entries = iType->second;
      for (size_t ei = 0, ee = entries.size(); ei != ee; ++ei)
      {
         const TableEntry &entry = entries[ei];
         columns.addValue(entry.name);
         columns.addValue(entry.expression);
         columns.addValue((snprintf(prec, 100, "%d", entry.precision), prec));
      }
      iTo.addKeyValue(typeName, columns);
   }
   iTo.addKeyValue(kConfigTypeNames, typeNames);
   */
}

void 
FWTableViewManager::setFrom(const FWConfiguration &iFrom)
{
   try
   {
      m_displayedCollection = iFrom.valueForKey("DisplayedCollection")->value();

      const FWConfiguration *typeNames = iFrom.valueForKey(kConfigTypeNames);
      if (typeNames == nullptr)
      {
         fwLog(fwlog::kWarning) << "no table column configuration stored, using defaults\n";
         return;
      }

      const FWConfiguration::KeyValues *keyValues = typeNames->keyValues();

      for (FWConfiguration::KeyValues::const_iterator iType = keyValues->begin(); iType != keyValues->end(); ++iType)
      {
         // std::cout << "reading type " << iType->first << std::endl;

         //	 const FWConfiguration *columns = iFrom.valueForKey(iType->first);
         const FWConfiguration *columns = &iType->second;
         assert(columns != nullptr);
         //   TableHandle handle = table(iType->first.c_str());
         for (FWConfiguration::StringValuesIt
                  it = columns->stringValues()->begin(),
                  itEnd = columns->stringValues()->end();
              it != itEnd; ++it)
         {
            const std::string &name = *it++;
            const std::string &expr = *it++;
            int prec = atoi(it->c_str());
            // std::cout << " name " << name.c_str() << "expre " << expr << std::endl;
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
