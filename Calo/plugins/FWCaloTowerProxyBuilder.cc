#include "TH2F.h"
#include "ROOT/REveCaloData.hxx"
#include "ROOT/REveViewContext.hxx"
#include "ROOT/REveSelection.hxx"
#include "ROOT/REveManager.hxx"
#include "ROOT/REveDataSimpleProxyBuilderTemplate.hxx"

// #include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/CaloTowers/interface/CaloTower.h"
#include "DataFormats/CaloTowers/interface/CaloTowerDefs.h"

#include "FireworksWeb/Core/interface/FWProxyBuilderFactory.h"
#include "FireworksWeb/Candidates/interface/CandidateUtils.h"
#include "FireworksWeb/Calo/interface/FWCaloTowerSliceSelector.h"
#include "FireworksWeb/Core/interface/FWWebEventItem.h"
#include "FireworksWeb/Core/interface/FWProxyBuilderConfiguration.h"
#include "FireworksWeb/Core/interface/Context.h"
#include "FireworksWeb/Tracks/interface/estimate_field.h"
#include "FireworksWeb/Core/interface/Context.h"
#include "FireworksWeb/Core/interface/FWMagField.h"
#include "FireworksWeb/Core/interface/fw3dlego_xbins.h"

using namespace ROOT::Experimental;
class FWCaloTowerProxyBuilderBase: public REveDataProxyBuilderBase
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



         auto fwitem = dynamic_cast<FWWebEventItem*>(Collection());
         std::string towerType = fwitem->purpose();
         caloData()->GetSelector()->AddSliceSelector(std::unique_ptr<REveCaloDataSliceSelector>
                                                    (new FWCaloTowerSliceSelector(fSliceIndex, Collection(), caloData(), towerType)));
      }
   }

public:
   virtual double getEt(const CaloTower*) { return 0;}

   REveCaloDataHist* caloData() { return fireworks::Context::getInstance()->getCaloData(); }
   
   using REveDataProxyBuilderBase::Build;
   void BuildProduct(const REveDataCollection* collection, REveElement* product, const REveViewContext*)override
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


         fHist->SetFillColor(Collection()->GetMainColor());
         for (int h = 0; h < collection->GetNItems(); ++h)
         {
            CaloTower* tower = (CaloTower*)(collection->GetDataPtr(h));
            const REveDataItem* item = Collection()->GetDataItem(h);

            if (!item->GetVisible())
               continue;

            fHist->Fill(tower->eta(), tower->phi(), getEt(tower));
         }
      }
      caloData()->DataChanged();
   }

   using REveDataProxyBuilderBase::FillImpliedSelected;
   void FillImpliedSelected(REveElement::Set_t& impSet, const std::set<int>& sec_idcs, Product*) override
   {
      caloData()->GetSelector()->SetActiveSlice(fSliceIndex);
      impSet.insert(caloData());
      caloData()->FillImpliedSelectedSet(impSet, sec_idcs);
   }

  using REveDataProxyBuilderBase::ModelChanges;
   void ModelChanges(const REveDataCollection::Ids_t& ids, Product* product) override
   {
      BuildProduct(Collection(), nullptr, nullptr);
   }
};

//-------------------------------------------------------------------------------
class FWECalCaloTowerProxyBuilder : public FWCaloTowerProxyBuilderBase
{
public:
   REGISTER_FWPB_METHODS();
  
   using FWCaloTowerProxyBuilderBase::getEt;
   double getEt(const CaloTower* t) override {
      return t->emEt();
   }
};

//-------------------------------------------------------------------------------
class FWHCalCaloTowerProxyBuilder : public FWCaloTowerProxyBuilderBase
{
public:
   REGISTER_FWPB_METHODS();
  
   using FWCaloTowerProxyBuilderBase::getEt;
   double getEt(const CaloTower* t) override {
      return t->hadEt();
   }
};
//-------------------------------------------------------------------------------
class FWHOCaloTowerProxyBuilder : public FWCaloTowerProxyBuilderBase
{
public:
   REGISTER_FWPB_METHODS();
  
   using FWCaloTowerProxyBuilderBase::getEt;
   double getEt(const CaloTower* t) override {
      return t->outerEt();
   }
};

REGISTER_FW2PROXYBUILDER(FWECalCaloTowerProxyBuilder,CaloTower,"ECal");
REGISTER_FW2PROXYBUILDER(FWHCalCaloTowerProxyBuilder,CaloTower,"HCal");
REGISTER_FW2PROXYBUILDER(FWHOCaloTowerProxyBuilder,CaloTower,"HCal Outer");
