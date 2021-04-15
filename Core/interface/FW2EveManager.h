#ifndef FireworksWeb_Core_FW2EveManager_h
#define FireworksWeb_Core_FW2EveManager_h

#include "ROOT/REveDataCollection.hxx"
#include "ROOT/REveViewContext.hxx"

#include "FireworksWeb/Core/interface/FW2Table.h"
#include "FireworksWeb/Core/interface/FWTypeToRepresentations.h"

class FWEventItem;
class FWTableViewManager;

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
   
   std::vector <ROOT::Experimental::REveScene*> m_scenes;
   ROOT::Experimental::REveViewContext* m_viewContext;
 
   ROOT::Experimental::REveProjectionManager* m_mngRhoZ;
   ROOT::Experimental::REveProjectionManager* m_mngRPhi;


   
   FWTableViewManager*   m_tableManager;
   
   std::vector<ROOT::Experimental::REveDataProxyBuilderBase*> m_builders;

   bool m_acceptChanges;

   typedef std::map<std::string,  std::vector<BuilderInfo> >  TypeToBuilder;
   TypeToBuilder            m_typeToBuilder;
    
public:
   FW2EveManager(FWTableViewManager* tableMng);
   void createScenesAndViews();
    void initTypeToBuilder();  

   void registerCollection(ROOT::Experimental::REveDataCollection* collection, bool table);
   void registerGraphicalProxy(ROOT::Experimental::REveDataCollection* collection, ROOT::Experimental::REveDataProxyBuilderBase* builder);
   /*
   TableHandle table(const char *collectionName){
      TableHandle handle(collectionName, m_tableFormats);
      return handle;
   }*/
   void endEvent();

   void beginEvent();

   void newItem(FWEventItem*);

   void modelChanged(ROOT::Experimental::REveDataItemList* collection, const ROOT::Experimental::REveDataCollection::Ids_t& ids);
   void FillImpliedSelected(ROOT::Experimental::REveDataItemList* itemList, ROOT::Experimental::REveElement::Set_t& impSelSet);

   FWTypeToRepresentations supportedTypesAndRepresentations() const;

};


#endif
