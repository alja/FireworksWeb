#ifndef Fireworks2_Core_FW2EveManager_h
#define Fireworks2_Core_FW2EveManager_h

#include "ROOT/REveDataCollection.hxx"
#include "ROOT/REveViewContext.hxx"

#include "Fireworks2/Core/interface/FW2Table.h"
#include "Fireworks2/Core/interface/FWTypeToRepresentations.h"

class FWEventItem;
class FWTableViewManager;

namespace ROOT::Experimental
{
class REveDataProxyBuilderBase;
class REveViewContext;
}

using namespace ROOT::Experimental;

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
   
   std::vector <REveScene*> m_scenes;
   REveViewContext* m_viewContext;
 
   REveProjectionManager* m_mngRhoZ;


   
   FWTableViewManager*   m_tableManager;
   
   std::vector<REveDataProxyBuilderBase*> m_builders;

   bool m_acceptChanges;

   typedef std::map<std::string,  std::vector<BuilderInfo> >  TypeToBuilder;
   TypeToBuilder            m_typeToBuilder;
   
    void initTypeToBuilder();   
public:
   FW2EveManager(FWTableViewManager* tableMng);
   void createScenesAndViews();

   void registerCollection(REveDataCollection* collection, bool table);
   void registerGraphicalProxy(REveDataCollection* collection, REveDataProxyBuilderBase* builder);
   /*
   TableHandle table(const char *collectionName){
      TableHandle handle(collectionName, m_tableFormats);
      return handle;
   }*/
   void endEvent();

   void beginEvent();

   void newItem(FWEventItem*);

   void modelChanged(REveDataItemList* collection, const REveDataCollection::Ids_t& ids);
   void FillImpliedSelected(REveDataItemList* itemList, REveElement::Set_t& impSelSet);

   FWTypeToRepresentations supportedTypesAndRepresentations() const;

};


#endif
