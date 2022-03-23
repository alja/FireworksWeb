#ifndef FireworksWeb_Core_FWAssociationManager_h
#define FireworksWeb_Core_FWAssociationManager_h

#include "ROOT/REveSelection.hxx"
#include "FireworksWeb/Core/interface/FWConfigurable.h"
#include "ROOT/REveDataCollection.hxx"

namespace ROOT { namespace Experimental {
    class REveScene;
}}

class FWAssociationProxyBase;

class FWAssociationManager : public FWConfigurable {
public:
   class FWSelectionDeviator : public ROOT::Experimental::REveSelection::Deviator
   {
      friend class FWAssociationManager;

   protected:
      void SelectAssociated(ROOT::Experimental::REveSelection *, ROOT::Experimental::REveDataItemList *);
   
   public:
      FWAssociationManager *m_eveMng;
      FWSelectionDeviator(FWAssociationManager *m) : m_eveMng(m) {}

      using ROOT::Experimental::REveSelection::Deviator::DeviateSelection;
      bool DeviateSelection(ROOT::Experimental::REveSelection *, ROOT::Experimental::REveElement *, bool, bool, const std::set<int> &);
   };

  FWAssociationManager();
  ~FWAssociationManager() override;

  void addTo(FWConfiguration&) const override;
  void setFrom(const FWConfiguration&) override;


   void initAssociations();
protected:
      std::vector<std::unique_ptr<FWAssociationProxyBase>> m_associations;

private:
   ROOT::Experimental::REveScene* m_scene {nullptr};
   std::shared_ptr<FWSelectionDeviator> m_selectionDeviator;
};

#endif
