#ifndef FWCANDIDATES_CANDIDATE_SELECT_H
#define FWCANDIDATES_CANDIDATE_SELECT_H

#include "ROOT/REveCaloData.hxx"
#include "ROOT/REveDataCollection.hxx"

class FWCaloDataCandidateSliceSelector : public ROOT::Experimental::REveCaloDataSliceSelector
{
private:
   ROOT::Experimental::REveDataCollection* fCollection{nullptr};
   ROOT::Experimental::REveCaloDataHist*   fCaloData{nullptr};

public:
   FWCaloDataCandidateSliceSelector(int s, ROOT::Experimental::REveDataCollection* c, ROOT::Experimental::REveCaloDataHist* h):REveCaloDataSliceSelector(s), fCollection(c), fCaloData(h) {}

   using ROOT::Experimental::REveCaloDataSliceSelector::ProcessSelection;
   void ProcessSelection(ROOT::Experimental::REveCaloData::vCellId_t& sel_cells, UInt_t selectionId, Bool_t multi) override;

   using ROOT::Experimental::REveCaloDataSliceSelector::GetCellsFromSecondaryIndices;
   void GetCellsFromSecondaryIndices(const std::set<int>& idcs, ROOT::Experimental::REveCaloData::vCellId_t& out) override;
};

#endif
