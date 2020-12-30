#include "TH2F.h"

#include "ROOT/REveCaloData.hxx"
#include "ROOT/REveViewContext.hxx"
#include "ROOT/REveSelection.hxx"
#include "ROOT/REveManager.hxx"
#include "ROOT/REveDataSimpleProxyBuilderTemplate.hxx"

#include "DataFormats/Candidate/interface/Candidate.h"

#include "Fireworks2/Core/interface/FWProxyBuilderFactory.h"
#include "Fireworks2/Candidates/interface/CandidateUtils.h"
#include "Fireworks2/Core/interface/FWEventItem.h"
#include "Fireworks2/Core/interface/FWProxyBuilderConfiguration.h"
#include "Fireworks2/Core/interface/Context.h"
#include "Fireworks2/Tracks/interface/estimate_field.h"
#include "Fireworks2/Core/interface/Context.h"
#include "Fireworks2/Core/interface/FWMagField.h"
#include "Fireworks2/Core/interface/fw3dlego_xbins.h"

using namespace ROOT::Experimental;

class CaloDataCandidateSliceSelector : public REveCaloDataSliceSelector
{
private:
   REveDataCollection* fCollection{nullptr};
   REveCaloDataHist*   fCaloData{nullptr};

public:
   //CaloDataCandidateSliceSelector(){}
   CaloDataCandidateSliceSelector(int s, REveDataCollection* c, REveCaloDataHist* h):REveCaloDataSliceSelector(s), fCollection(c), fCaloData(h) {}

   using REveCaloDataSliceSelector::ProcessSelection;
   void ProcessSelection(REveCaloData::vCellId_t& sel_cells, UInt_t selectionId, Bool_t multi) override
   {
      std::set<int> item_set;
      REveCaloData::CellData_t cd;
      for (auto &cellId : sel_cells)
      {
         fCaloData->GetCellData(cellId, cd);

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
      sel->NewElementPicked(fCollection->GetItemList()->GetElementId(),  multi, true, item_set);
   }

   using REveCaloDataSliceSelector::GetCellsFromSecondaryIndices;
   void GetCellsFromSecondaryIndices(const std::set<int>& idcs, REveCaloData::vCellId_t& out) override
   {
      TH2F* hist  =  fCaloData->GetHist(GetSliceIndex());
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
};



class FWCandidateTowerProxyBuilder: public REveDataProxyBuilderBase
{
private:
   REveCaloDataHist* fCaloData {nullptr};
   TH2F*             fHist {nullptr};
   int               fSliceIndex {-1};

   void assertSlice() {
      if (!fHist) {
         Bool_t status = TH1::AddDirectoryStatus();

         TH1::AddDirectory(kFALSE);  //Keeps histogram from going into memory
         fHist = new TH2F("caloHist", "caloHist", fw3dlego::xbins_n - 1, fw3dlego::xbins, 72, -M_PI, M_PI);
         TH1::AddDirectory(status);
         fSliceIndex = fCaloData->AddHistogram(fHist);

         fCaloData->RefSliceInfo(fSliceIndex)
            .Setup(Collection()->GetCName(),
                   0.,
                   Collection()->GetMainColor(),
                   Collection()->GetMainTransparency());

         fCaloData->GetSelector()->AddSliceSelector(std::unique_ptr<REveCaloDataSliceSelector>
                                                    (new CaloDataCandidateSliceSelector(fSliceIndex, Collection(), fCaloData)));
      }
   }

public:
   REGISTER_FWPB_METHODS();

   FWCandidateTowerProxyBuilder() {
      fCaloData = fireworks::Context::getInstance()->getCaloData();
      /*
      if (!fCaloData->GetSelector())
      {
         auto selector = new CaloDataCandidateSelector(fCaloData);
         fCaloData->SetSelector(selector);
      }
      */
   }

   using REveDataProxyBuilderBase::Build;
   void Build(const REveDataCollection* collection, REveElement* product, const REveViewContext*)override
   {
      assertSlice();
      fHist->Reset();
      if (collection->GetRnrSelf())
      {
         fCaloData->RefSliceInfo(fSliceIndex)
            .Setup(Collection()->GetCName(),
                   0.,
                   Collection()->GetMainColor(),
                   Collection()->GetMainTransparency());


         for (int h = 0; h < collection->GetNItems(); ++h)
         {
            reco::Candidate* tower = (reco::Candidate*)(collection->GetDataPtr(h));
            const REveDataItem* item = Collection()->GetDataItem(h);

            if (!item->GetVisible())
               continue;
               
            fHist->Fill(tower->eta(), tower->phi(), tower->et());
         }
      }
      fCaloData->DataChanged();
   }

   using REveDataProxyBuilderBase::FillImpliedSelected;
   void FillImpliedSelected(REveElement::Set_t& impSet, Product*) override
   {
      fCaloData->GetSelector()->SetActiveSlice(fSliceIndex);
      impSet.insert(fCaloData);
      fCaloData->FillImpliedSelectedSet(impSet);
   }

  using REveDataProxyBuilderBase::ModelChanges;
   void ModelChanges(const REveDataCollection::Ids_t& ids, Product* product) override
   {
      Build();
   }
};

REGISTER_FW2PROXYBUILDER(FWCandidateTowerProxyBuilder, reco::Candidate, "CandidateTowers");
