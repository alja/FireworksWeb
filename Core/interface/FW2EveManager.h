#ifndef FireworksWeb_Core_FW2EveManager_h
#define FireworksWeb_Core_FW2EveManager_h


#include "FireworksWeb/Core/interface/FWTypeToRepresentations.h"
#include "ROOT/REveDataCollection.hxx"
#include "ROOT/REveSelection.hxx"

class FWEventItem;
class FWTableViewManager;
class FWEveView;
class FWAssociationBase;

namespace ROOT
{
   namespace Experimental
   {
      class REveDataProxyBuilderBase;
      class REveViewContext;
   }
}

class FW2EveManager
{
public:
   class FWSelectionDeviator : public ROOT::Experimental::REveSelection::Deviator
   {
    
      friend class FW2EveManager;

   protected:
      void SelectAssociated(ROOT::Experimental::REveSelection *, ROOT::Experimental::REveDataItemList *);

   public:
      FW2EveManager *m_eveMng;
      FWSelectionDeviator(FW2EveManager *m) : m_eveMng(m) {}

      using ROOT::Experimental::REveSelection::Deviator::DeviateSelection;
      bool DeviateSelection(ROOT::Experimental::REveSelection *, ROOT::Experimental::REveElement *, bool, bool, const std::set<int> &);
   };

private:
   
   struct BuilderInfo
   {
      std::string m_name;

      void classType(std::string& , bool&) const;

      BuilderInfo(std::string name) :
         m_name(name)
      {}
   };
   std::vector<FWEveView*> m_views;
   ROOT::Experimental::REveViewContext* m_viewContext{nullptr};
   
   FWTableViewManager*   m_tableManager{nullptr};
   
   std::vector<ROOT::Experimental::REveDataProxyBuilderBase*> m_builders;

   bool m_acceptChanges{true};

   typedef std::map<std::string,  std::vector<BuilderInfo> >  TypeToBuilder;
   TypeToBuilder            m_typeToBuilder;

   std::shared_ptr<FWSelectionDeviator> m_selectionDeviator;

   protected:
      std::vector<std::unique_ptr<FWAssociationBase>> m_associations;
    
public:
   FW2EveManager(FWTableViewManager* tableMng);
   void createScenesAndViews();
    void initTypeToBuilder();  
   void initAssociations();

   void addGraphicalProxyBuilder(ROOT::Experimental::REveDataCollection* collection, ROOT::Experimental::REveDataProxyBuilderBase* builder);
   void addTableProxyBuilder(ROOT::Experimental::REveDataCollection*);

   void endEvent();
   void beginEvent();

   void newItem(FWEventItem*);
   void globalEnergyScaleChanged();

   void modelChanged(ROOT::Experimental::REveDataItemList* collection, const ROOT::Experimental::REveDataCollection::Ids_t& ids);
   void FillImpliedSelected(ROOT::Experimental::REveDataItemList* itemList, ROOT::Experimental::REveElement::Set_t& impSelSet, const std::set<int>& sec_idcs);

   FWTypeToRepresentations supportedTypesAndRepresentations() const;

  // void DeviateCollectionSelection(ROOT::Experimental::REveSelection* selection, ROOT::Experimental::REveDataItemList* col, bool multi, bool secondary, const std::set<int>& secondary_idcs);
};

#endif
