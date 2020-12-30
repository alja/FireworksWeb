#include "TH2F.h"
#include "ROOT/REveCaloData.hxx"
#include "ROOT/REveViewContext.hxx"
#include "ROOT/REveSelection.hxx"
#include "ROOT/REveManager.hxx"
#include "ROOT/REveDataSimpleProxyBuilderTemplate.hxx"

#include "DataFormats/Candidate/interface/Candidate.h"

#include "Fireworks2/Core/interface/FWProxyBuilderFactory.h"
#include "Fireworks2/Candidates/interface/CandidateUtils.h"
#include "Fireworks2/Candidates/interface/FWCandidateSliceSelector.h"
#include "Fireworks2/Core/interface/FWEventItem.h"
#include "Fireworks2/Core/interface/FWProxyBuilderConfiguration.h"
#include "Fireworks2/Core/interface/Context.h"
#include "Fireworks2/Tracks/interface/estimate_field.h"
#include "Fireworks2/Core/interface/Context.h"
#include "Fireworks2/Core/interface/FWMagField.h"
#include "Fireworks2/Core/interface/fw3dlego_xbins.h"

using namespace ROOT::Experimental;
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
                                                    (new FWCaloDataCandidateSliceSelector(fSliceIndex, Collection(), fCaloData)));
      }
   }

public:
   REGISTER_FWPB_METHODS();

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
