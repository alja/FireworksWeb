#include "TH2F.h"

#include "FireworksWeb/Candidates/interface/FWCandidateSliceSelector.h"
#include "FireworksWeb/Core/interface/Context.h"
#include "ROOT/REveSelection.hxx"
#include "ROOT/REveManager.hxx"
#include "DataFormats/Candidate/interface/Candidate.h"

using namespace ROOT::Experimental;

REveCaloDataHist*
FWCaloDataCandidateSliceSelector::caloData()
{
   return fireworks::Context::getInstance()->getCaloData();
}

void
FWCaloDataCandidateSliceSelector::ProcessSelection(REveCaloData::vCellId_t& sel_cells, UInt_t selectionId, Bool_t multi)
{
   std::set<int> item_set;
   REveCaloData::CellData_t cd;
   for (auto &cellId : sel_cells)
   {
      caloData()->GetCellData(cellId, cd);

      // loop over enire collection and check its eta/phi range
      for (int t = 0; t < fCollection->GetNItems(); ++t)
      {
         reco::Candidate* tower = (reco::Candidate*) fCollection->GetDataPtr(t);
         if (tower->eta() > cd.fEtaMin && tower->eta() < cd.fEtaMax &&
             tower->phi() > cd.fPhiMin && tower->phi() < cd.fPhiMax)
            item_set.insert(t);
      }
   }
   REveSelection* sel = (REveSelection*)gEve->FindElementById(selectionId);
   fCollection->GetItemList()->RefSelectedSet() = item_set;
   sel->NewElementPicked(fCollection->GetItemList()->GetElementId(),  multi, true, item_set);
}

void
FWCaloDataCandidateSliceSelector::GetCellsFromSecondaryIndices(const std::set<int>& idcs, REveCaloData::vCellId_t& out)
{
   TH2F* hist  =  caloData()->GetHist(GetSliceIndex());
   std::set<int> cbins;
   // float total = 0;
   for( auto &i : idcs ) {
      reco::Candidate* tower = (reco::Candidate*)fCollection->GetDataPtr(i);
      int bin = hist->FindBin(tower->eta(), tower->phi());
      float frac =  tower->et()/hist->GetBinContent(bin);
      bool ex = false;
      for (size_t ci = 0; ci < out.size(); ++ci)
      {
         if (out[ci].fTower == bin && out[ci].fSlice == GetSliceIndex())
         {
            float oldv =  out[ci].fFraction;
            out[ci].fFraction = oldv + frac;
            ex = true;
            break;
         }
      }
      if (!ex) {
         out.push_back(REveCaloData::CellId_t(bin, GetSliceIndex(), frac));
      }
   }
}
