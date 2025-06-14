#ifndef FireworksWeb_Core_FW2EveManager_h
#define FireworksWeb_Core_FW2EveManager_h


#include "FireworksWeb/Core/interface/FWTypeToRepresentations.h"
#include "ROOT/REveDataCollection.hxx"

class FWWebEventItem;
class FWTableViewManager;
class FWEveView;

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
    
public:
   FW2EveManager(FWTableViewManager* tableMng);
   void createScenesAndViews(std::string&);
    void initTypeToBuilder();  

   void addGraphicalProxyBuilder(ROOT::Experimental::REveDataCollection* collection, ROOT::Experimental::REveDataProxyBuilderBase* builder);
   void addTableProxyBuilder(ROOT::Experimental::REveDataCollection*);
    
    void itemConfigChanged(FWWebEventItem*);

   void endEvent();
   void beginEvent();

   void newItem(FWWebEventItem*);
   void globalEnergyScaleChanged();
   void globalBackgroundChanged(bool);

   void modelChanged(ROOT::Experimental::REveDataItemList* collection, const ROOT::Experimental::REveDataCollection::Ids_t& ids);
   void FillImpliedSelected(ROOT::Experimental::REveDataItemList* itemList, ROOT::Experimental::REveElement::Set_t& impSelSet, const std::set<int>& sec_idcs);

   FWTypeToRepresentations supportedTypesAndRepresentations() const;

  // void DeviateCollectionSelection(ROOT::Experimental::REveSelection* selection, ROOT::Experimental::REveDataItemList* col, bool multi, bool secondary, const std::set<int>& secondary_idcs);
};

#endif
