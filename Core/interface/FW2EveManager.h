#ifndef Fireworks2_Core_FW2EveManager_h
#define Fireworks2_Core_FW2EveManager_h

#include "ROOT/REveDataClasses.hxx"
#include "ROOT/REveViewContext.hxx"

#include "OssTests/Core/interface/FW2Table.h"
namespace ROOT::Experimental
{
class REveDataProxyBuilderBase;
class REveViewContext;
}

namespace REX = ROOT::Experimental;

class FW2EveManager
{
private:   
   std::vector <REX::REveScene*> m_scenes;
   REX::REveViewContext* m_viewContext;

   REX::REveProjectionManager* m_mngRhoZ;

   std::vector<REX::REveDataProxyBuilderBase*> m_builders;

   TableHandle::TableSpecs  m_tableFormats;

   std::string m_tableCollection;
   
public:
   FW2EveManager();
   void createScenesAndViews();
   
   void registerCollection(REX::REveDataCollection* collection, REX::REveDataProxyBuilderBase* builder, bool table);

   TableHandle table(const char *collectionName){
      TableHandle handle(collectionName, m_tableFormats);
      return handle;
   }
   void eventChanged();

   void setTableCollection(const char* name) { m_tableCollection = name; }

   void modelChanged(REX::REveDataCollection* collection, const REX::REveDataCollection::Ids_t& ids);
};


#endif
