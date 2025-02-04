#ifndef FWCALO_CALO_SELECT_H
#define FWCALO_CALO_SELECT_H

#include "ROOT/REveCaloData.hxx"
#include "ROOT/REveDataCollection.hxx"

class FWCaloTowerSliceSelector : public ROOT::Experimental::REveCaloDataSliceSelector
{
private:
   ROOT::Experimental::REveDataCollection* fCollection{nullptr};
   ROOT::Experimental::REveCaloDataHist* caloData();

   std::string m_towerType;
public:
   FWCaloTowerSliceSelector(int s, ROOT::Experimental::REveDataCollection* c, ROOT::Experimental::REveCaloDataHist* h, const std::string& tt):REveCaloDataSliceSelector(s), fCollection(c), m_towerType(tt) {}

   using ROOT::Experimental::REveCaloDataSliceSelector::ProcessSelection;
   void ProcessSelection(ROOT::Experimental::REveCaloData::vCellId_t& sel_cells, UInt_t selectionId, Bool_t multi) override;

   using ROOT::Experimental::REveCaloDataSliceSelector::GetCellsFromSecondaryIndices;
   void GetCellsFromSecondaryIndices(const std::set<int>& idcs, ROOT::Experimental::REveCaloData::vCellId_t& out) override;

};

#endif
