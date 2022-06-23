
#ifndef FireworksWeb_Core_FWDigitSetProxyBuilder_h
#define FireworksWeb_Core_FWDigitSetProxyBuilder_h

#include "ROOT/REveDataProxyBuilderBase.hxx"
#include "FireworksWeb/Core/interface/FWDisplayProperties.h"

#include "ROOT/REveBoxSet.hxx"

namespace ROOT
{
  namespace Experimental
  {
    class REveBoxSet;
    class REveDataCollection;
    class REveDataItem;
    class REveElement;
    class REveViewContext;
  }
}

class FWBoxSet : public ROOT::Experimental::REveBoxSet
{
public:
  using ROOT::Experimental::REveElement::GetSelectionMaster;
  ROOT::Experimental::REveElement *GetSelectionMaster() override;
};

class FWDigitSetProxyBuilder : public ROOT::Experimental::REveDataProxyBuilderBase
{
public:
  FWDigitSetProxyBuilder();
  ~FWDigitSetProxyBuilder() override;

protected:
  ROOT::Experimental::REveBoxSet* addBoxSetToProduct(ROOT::Experimental::REveElement *product);
  void addBox(ROOT::Experimental::REveBoxSet *set, const float *pnts, const ROOT::Experimental::REveDataItem*);
  ROOT::Experimental::REveBoxSet *m_boxSet;

private:
  FWDigitSetProxyBuilder(const FWDigitSetProxyBuilder &) = delete; // stop default

  const FWDigitSetProxyBuilder &operator=(const FWDigitSetProxyBuilder &) = delete; // stop default

  // ---------- member data --------------------------------

  //using ROOT::Experimental::REveDataProxyBuilderBase::Build;
  //void BuildProduct(const ROOT::Experimental::REveDataCollection *collection, ROOT::Experimental::REveElement *product, const ROOT::Experimental::REveViewContext*) override;

  using ROOT::Experimental::REveDataProxyBuilderBase::FillImpliedSelected;
  void FillImpliedSelected(ROOT::Experimental::REveElement::Set_t &impSet, const std::set<int> &sec_idcs, Product *p) override;

  using ROOT::Experimental::REveDataProxyBuilderBase::ModelChanges;
  void ModelChanges(const ROOT::Experimental::REveDataCollection::Ids_t &ids, Product *product) override;

   // reuse the same boxes throug changes in collection
  using REveDataProxyBuilderBase::Clean;
  void Clean() override {};
};

#endif
