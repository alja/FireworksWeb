#ifndef FireworksWeb_Core_FWAssociationManager_h
#define FireworksWeb_Core_FWAssociationManager_h

#include "ROOT/REveSelection.hxx"
#include "FireworksWeb/Core/interface/FWConfigurable.h"
#include "ROOT/REveDataCollection.hxx"

namespace ROOT { namespace Experimental {
    class REveScene;
}}

class FWAssociationProxyBase;
class FWPhysicsObjectDesc; 

class FWAssociationManager : public FWConfigurable {
public:
   class FWSelectionDeviator : public ROOT::Experimental::REveSelection::Deviator
   {
      friend class FWAssociationManager;

   protected:
      void SelectAssociated(ROOT::Experimental::REveSelection *, ROOT::Experimental::REveDataItemList *);
      ROOT::Experimental::REveDataItemList* m_selected{nullptr};
   
   public:
      FWAssociationManager *m_mng;
      FWSelectionDeviator(FWAssociationManager *m) : m_mng(m) {}

      using ROOT::Experimental::REveSelection::Deviator::DeviateSelection;
      bool DeviateSelection(ROOT::Experimental::REveSelection *, ROOT::Experimental::REveElement *, bool, bool, const std::set<int> &);
   };

  FWAssociationManager();
  ~FWAssociationManager() override;

  void addTo(FWConfiguration&) const override;
  void setFrom(const FWConfiguration&) override;
  void filterChanged();
  void refAssociationTypes(std::vector< std::string >&) const;

  void addAssociation(FWPhysicsObjectDesc& d);

protected:
      std::vector<std::unique_ptr<FWAssociationProxyBase>> m_associations;

private:
   ROOT::Experimental::REveScene* m_scene {nullptr};
   std::shared_ptr<FWSelectionDeviator> m_selectionDeviator;

   void addAssociationInternal(const std::string &name, const std::string &type,
                       const std::string &moduleLabel, const std::string &productInstanceLabel,
                       const std::string &processName, const std::string &filterExpression);
};

#endif
