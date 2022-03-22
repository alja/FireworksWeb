#include "FireworksWeb/Core/interface/FWAssociationManager.h"
#include "FireworksWeb/Core/interface/FWEveAssociation.h"

#include "ROOT/REveDataCollection.hxx"
#include "ROOT/REveScene.hxx"
#include "ROOT/REveManager.hxx"

#include "FireworksWeb/Core/interface/FWAssociationProxyBase.h"
#include "FireworksWeb/Core/interface/FWAssociationFactory.h"

using namespace ROOT::Experimental;

FWAssociationManager::FWAssociationManager() {

   m_selectionDeviator = std::make_shared<FWSelectionDeviator>(this);
   gEve->GetSelection()->SetDeviator( m_selectionDeviator);

   gEve->GetHighlight()->SetDeviator( m_selectionDeviator);
}

FWAssociationManager::~FWAssociationManager() {}



void FWAssociationManager::addTo(FWConfiguration& iTo) const {
    /*
  for (std::vector<FWAssociation*>::const_iterator it = m_associations.begin(); it != m_associations.end(); ++it) {
    if (!*it)
      continue;
    FWConfiguration conf(6);
    edm::TypeWithDict dataType((*((*it)->type()->GetTypeInfo())));
    assert(dataType != edm::TypeWithDict());

    conf.addKeyValue(kType, FWConfiguration(dataType.name()));
    conf.addKeyValue(kModuleLabel, FWConfiguration((*it)->moduleLabel()));
    conf.addKeyValue(kProductInstanceLabel, FWConfiguration((*it)->productInstanceLabel()));
    conf.addKeyValue(kProcessName, FWConfiguration((*it)->processName()));
    conf.addKeyValue(kFilterExpression, FWConfiguration((*it)->filterExpression()));
  
    conf.addKeyValue(kIsVisible, FWConfiguration((*it)->getCollection()->GetRnrSelf() ? kTrue : kFalse));
    {
      std::ostringstream os;
      // os << (*it)->layer();
      os << 1;
      conf.addKeyValue(kLayer, FWConfiguration(os.str()));
    }
    iTo.addKeyValue((*it)->name(), conf, true);
  }*/
}

void FWAssociationManager::setFrom(const FWConfiguration& iFrom) {
    /*
  const FWConfiguration::KeyValues* keyValues = iFrom.keyValues();

  if (keyValues == nullptr)
    return;
    */
}


//______________________________________________________________________________
void FWAssociationManager::initAssociations()
{
   try
   {
      if (edmplugin::PluginManager::get()->categoryToInfos().end() != edmplugin::PluginManager::get()->categoryToInfos().find(FWAssociationFactory::get()->category()))
      {
         std::vector<edmplugin::PluginInfo> ac = edmplugin::PluginManager::get()->categoryToInfos().find(FWAssociationFactory::get()->category())->second;
         for (auto &i : ac)
         {
            std::string pn = i.name_;

            printf("instantiating Association %s \n", pn.c_str());

          //  auto builder = FWAssociationFactory::get()->create(pn);
            m_associations.push_back(FWAssociationFactory::get()->create(pn));
            std::cout << "associatable .... " << m_associations.back()->associatable() << " associated " << m_associations.back()->associated() << std::endl;
         }
      }
   }
   catch (std::exception &e)
   {
      std::cout << "Erro in FW2EveManager::initAssoications() " << e.what() << "\n";
   }
}
//__________________________________________________________________________________
void FWAssociationManager::FWSelectionDeviator::SelectAssociated(REveSelection *selection, REveDataItemList *colItems)
{
   REveDataCollection *ac = static_cast<REveDataCollection *>(colItems->GetMother());

   REveElement *collectionList = ac->GetMother();
   std::string itemClass = ac->GetItemClass()->GetName();
   for (auto &ap : m_eveMng->m_associations)
   {
      if (ap->associatable() == itemClass)
      {
         std::string associatedClassName = ap->associated();
         for (auto &cc : collectionList->RefChildren())
         {
            REveDataCollection *candCol = static_cast<REveDataCollection *>(cc);
            std::string x = candCol->GetItemClass()->GetName();
            if (x == associatedClassName)
            {
               std::set<int> iset;
               ap->getIndices(colItems->RefSelectedSet(), iset);

               std::cout << "selecting associated through plugin " << candCol->GetName() << " idcs size " << iset.size() << "\n";
               ExecuteNewElementPicked(selection, candCol->GetItemList(), true, true, iset);
            }
         }
      }
   }
}

//__________________________________________________________________________________
bool FWAssociationManager::FWSelectionDeviator::DeviateSelection(REveSelection *selection, REveElement *el, bool multi, bool secondary, const std::set<int> &secondary_idcs)
{  
   if (el)
   {
      auto *colItems = dynamic_cast<REveDataItemList *>(el);
      if (colItems)
      {
         // std::cout << "Deviate " << colItems->RefSelectedSet().size() << " passed set " << secondary_idcs.size() << "\n";
         ExecuteNewElementPicked(selection, colItems, multi, true, colItems->RefSelectedSet());
          
          if (selection == gEve->GetSelection())
              SelectAssociated(selection, colItems);

         return true;
      }
   }
   return false;
}
