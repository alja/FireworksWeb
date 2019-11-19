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
#include <boost/bind.hpp>
#include <algorithm>
#include <ROOT/REveTableInfo.hxx>

#include "TClass.h"
#include "FWCore/Reflection/interface/BaseWithDict.h"
#include "FWCore/Reflection/interface/MemberWithDict.h"
#include "FWCore/Reflection/interface/FunctionWithDict.h"

// user include files
#include "Fireworks2/Core/interface/FWTableViewManager.h"
#include "Fireworks2/Core/interface/FWConfiguration.h"
#include "Fireworks2/Core/interface/FWEventItem.h"
#include "Fireworks/Core/interface/FWTypeToRepresentations.h"
#include "Fireworks2/Core/interface/fwLog.h"

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
kConfigTypeNames

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

      
   m_tableInfo->table("reco::PFMET").
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
      column("endcap", 0, "i.cscDetId.endcap()").
      column("station", 0, "i.cscDetId.station()").
      column("ring", 0, "i.cscDetId.ring()").
      column("chamber", 0, "i.cscDetId.chamber()");

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

      m_displayedCollection =  iFrom.valueForKey("DisplayedCollection")->value();
       
      /*
      const FWConfiguration *typeNames = iFrom.valueForKey(kConfigTypeNames);
      if (typeNames == nullptr)
      {
         fwLog(fwlog::kWarning) << "no table column configuration stored, using defaults\n";
         return;
      }
            
      //NOTE: FWTableViewTableManagers hold pointers into m_tableFormats so if we
      // clear it those pointers would be invalid
      // instead we will just clear the lists and fill them with their new values
      //m_tableFormats.clear();
      for (FWConfiguration::StringValuesIt 
	   iType = typeNames->stringValues()->begin(),
	   iTypeEnd = typeNames->stringValues()->end(); 
           iType != iTypeEnd; ++iType) 
      {
         //std::cout << "reading type " << *iType << std::endl;
	 const FWConfiguration *columns = iFrom.valueForKey(*iType);
	 assert(columns != nullptr);
         TableHandle handle = table(iType->c_str());
	 for (FWConfiguration::StringValuesIt 
	      it = columns->stringValues()->begin(),
	      itEnd = columns->stringValues()->end(); 
	      it != itEnd; ++it) 
         {
	    const std::string &name = *it++;
	    const std::string &expr = *it++;
	    int prec = atoi(it->c_str());
            handle.column(name.c_str(), prec, expr.c_str());
	 }
      }
      */
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
}
