
#include "FireworksWeb/Core/interface/FWDigitSetProxyBuilder.h"


#include <assert.h>
using namespace ROOT::Experimental;

ROOT::Experimental::REveElement *FWBoxSet::GetSelectionMaster()
{
  if (fSelectionMaster)
  {
    REveDataItemList *il = dynamic_cast<REveDataItemList *>(fSelectionMaster);
    il->RefSelectedSet() = RefSelectedSet();
    return il;
  }
  return nullptr;
}

FWDigitSetProxyBuilder::FWDigitSetProxyBuilder() : m_boxSet(nullptr) {}

FWDigitSetProxyBuilder::~FWDigitSetProxyBuilder() {
  m_boxSet->DecDenyDestroy();
}


REveBoxSet* FWDigitSetProxyBuilder::addBoxSetToProduct(REveElement* product) {
  m_boxSet = new FWBoxSet();
  m_boxSet->SetPickable(true);
  m_boxSet->SetAlwaysSecSelect(true);
  m_boxSet->IncDenyDestroy();
  m_boxSet->SetSelectionMaster(Collection()->GetItemList());
  product->AddElement(m_boxSet);
  return m_boxSet;
}

void FWDigitSetProxyBuilder::addBox(REveBoxSet* boxSet, const float* pnts, const REveDataItem* di) {
  boxSet->AddBox(pnts);
  boxSet->DigitValue(di->GetVisible());

  if (di->GetVisible())
    boxSet->DigitColor(di->GetMainColor());

  //if (dp.transparency())
   // boxSet->SetMainTransparency(dp.transparency()); 
}

void FWDigitSetProxyBuilder::FillImpliedSelected(REveElement::Set_t &impSet, const std::set<int> &sec_idcs, Product *p)
{
  //  printf("RecHit fill implioed ----------------- !!!%zu\n", Collection()->GetItemList()->RefSelectedSet().size());
  impSet.insert(m_boxSet);
}

void FWDigitSetProxyBuilder::ModelChanges(const REveDataCollection::Ids_t &ids, Product *product)
{
  // We know there is only one element in this product
  printf("FWDigitSetProxyBuilder::ModelChanges %zu\n", ids.size());

  for (auto &i : ids)
  {
    auto item = Collection()->GetDataItem(i);
    m_boxSet->SetCurrentDigit(i);
    if (item->GetVisible())
    {
      m_boxSet->DigitValue(1);
      m_boxSet->DigitColor(item->GetMainColor());
    }
    else
    {
      m_boxSet->DigitValue(0);
    }
  }

  m_boxSet->StampObjProps();
}
