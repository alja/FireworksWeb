#include "TH2F.h"
#include "ROOT/REveCaloData.hxx"
#include "ROOT/REveViewContext.hxx"
#include "ROOT/REveSelection.hxx"
#include "ROOT/REveManager.hxx"
#include "ROOT/REveDataSimpleProxyBuilderTemplate.hxx"

#include "DataFormats/Candidate/interface/Candidate.h"

#include "FireworksWeb/Core/interface/FWProxyBuilderFactory.h"
#include "FireworksWeb/Candidates/interface/CandidateUtils.h"
#include "FireworksWeb/Candidates/interface/FWCandidateSliceSelector.h"
#include "FireworksWeb/Core/interface/FWEventItem.h"
#include "FireworksWeb/Core/interface/FWProxyBuilderConfiguration.h"
#include "FireworksWeb/Core/interface/Context.h"
#include "FireworksWeb/Tracks/interface/estimate_field.h"
#include "FireworksWeb/Core/interface/Context.h"
#include "FireworksWeb/Core/interface/FWMagField.h"
#include "FireworksWeb/Core/interface/fw3dlego_xbins.h"

using namespace ROOT::Experimental;
class FWCandidateTowerProxyBuilder: public REveDataProxyBuilderBase
{
private:
   TH2F*             fHist {nullptr};
   int               fSliceIndex {-1};

   void assertSlice() {
      if (!fHist) {
         Bool_t status = TH1::AddDirectoryStatus();

         TH1::AddDirectory(kFALSE);  //Keeps histogram from going into memory
         fHist = new TH2F("caloHist", "caloHist", fw3dlego::xbins_n - 1, fw3dlego::xbins, 72, -M_PI, M_PI);
         TH1::AddDirectory(status);
         fSliceIndex = caloData()->AddHistogram(fHist);

         caloData()->RefSliceInfo(fSliceIndex)
            .Setup(Collection()->GetCName(),
                   0.,
                   Collection()->GetMainColor(),
                   Collection()->GetMainTransparency());

         caloData()->GetSelector()->AddSliceSelector(std::unique_ptr<REveCaloDataSliceSelector>
                                                    (new FWCaloDataCandidateSliceSelector(fSliceIndex, Collection(), caloData())));
      }
   }


   REveCaloDataHist* caloData() {return fireworks::Context::getInstance()->getCaloData();}

public:
   REGISTER_FWPB_METHODS();

   using REveDataProxyBuilderBase::Build;
   void Build(const REveDataCollection* collection, REveElement* product, const REveViewContext*)override
   {
      assertSlice();
      fHist->Reset();
      if (collection->GetRnrSelf())
      {
         caloData()->RefSliceInfo(fSliceIndex)
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
      caloData()->DataChanged();
   }

   using REveDataProxyBuilderBase::FillImpliedSelected;
   void FillImpliedSelected(REveElement::Set_t& impSet, Product*) override
   {
      caloData()->GetSelector()->SetActiveSlice(fSliceIndex);
      impSet.insert(caloData());
      caloData()->FillImpliedSelectedSet(impSet);
   }

  using REveDataProxyBuilderBase::ModelChanges;
   void ModelChanges(const REveDataCollection::Ids_t& ids, Product* product) override
   {
      Build();
   }
};

REGISTER_FW2PROXYBUILDER(FWCandidateTowerProxyBuilder, reco::Candidate, "CandidateTowers");
