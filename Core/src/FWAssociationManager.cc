
#include "ROOT/REveDataCollection.hxx"
#include "ROOT/REveScene.hxx"
#include "ROOT/REveManager.hxx"

#include "FireworksWeb/Core/interface/FWAssociationProxyBase.h"
#include "FireworksWeb/Core/interface/FWAssociationFactory.h"
#include "FireworksWeb/Core/interface/FWConfiguration.h"
#include "FireworksWeb/Core/interface/FWAssociationManager.h"
#include "FireworksWeb/Core/interface/FWEveAssociation.h"

#include "FWCore/Reflection/interface/TypeWithDict.h"

static const std::string kType("type");
static const std::string kModuleLabel("moduleLabel");
static const std::string kProductInstanceLabel("productInstanceLabel");
static const std::string kProcessName("processName");
static const std::string kFilterExpression("filterExpression");

using namespace ROOT::Experimental;

FWAssociationManager::FWAssociationManager() {

   m_selectionDeviator = std::make_shared<FWSelectionDeviator>(this);

   gEve->GetSelection()->SetDeviator( m_selectionDeviator);
   gEve->GetHighlight()->SetDeviator( m_selectionDeviator);

   m_scene = ROOT::Experimental::gEve->SpawnNewScene("Associations", "Associations");

}

FWAssociationManager::~FWAssociationManager() {}

/*
void FWAssociationManager::initAssociations()
{
    FWEveAssociation *a = new FWEveAssociation("RecoToSimAssociation", TClass::GetClass("hgcal::RecoToSimCollection"), "layerClusterCaloParticleAssociationProducer");
    m_scene->AddElement(a);
    FWEveAssociation *b = new FWEveAssociation("SimToRecoAssociation", TClass::GetClass("hgcal::SimToRecoCollection"), "layerClusterCaloParticleAssociationProducer");
    m_scene->AddElement(b);

    try
    {
        for (auto &cc : m_scene->RefChildren())
        {
            FWEveAssociation *eveAssociation = (FWEveAssociation *)(cc);
            std::string typeName = eveAssociation->m_type->GetName();
            std::vector<edmplugin::PluginInfo> ac = edmplugin::PluginManager::get()->categoryToInfos().find(FWAssociationFactory::get()->category())->second;

           // std::cout << "\n============= eve ass Look match for tpe " << typeName << "\n";

            edm::TypeWithDict modelType(*(eveAssociation->m_type->GetTypeInfo()));
            std::string atn = modelType.typeInfo().name();

            for (auto &i : ac) // loop plugins
            {
                std::string pnh = i.name_;
                std::string pn = pnh.substr(0, pnh.find_first_of('@'));

                if (atn == pn)
                {
                    m_associations.push_back(FWAssociationFactory::get()->create(pnh));
                    std::cout << "associatable .... " << m_associations.back()->associatable() << " associated " << m_associations.back()->associated() << std::endl;
                    m_associations.back()->setEveObj(eveAssociation);
                    break;
                }
            }
        }
    }
    catch (std::exception &e)
    {
        std::cout << "Erro in FW2EveManager::initAssoications() " << e.what() << "\n";
    }
}*/

//______________________________________________________________________________
void FWAssociationManager::addTo(FWConfiguration &iTo) const
{
    for (auto &c : m_scene->RefChildren())
    {
        FWEveAssociation *a = (FWEveAssociation *)(c);

        FWConfiguration conf(6);
        edm::TypeWithDict dataType(*(a->m_type->GetTypeInfo()));
        assert(dataType != edm::TypeWithDict());

        conf.addKeyValue(kType, FWConfiguration(dataType.name()));
        conf.addKeyValue(kModuleLabel, FWConfiguration(a->m_moduleLabel));
        conf.addKeyValue(kProductInstanceLabel, FWConfiguration(a->m_productInstanceLabel));
        conf.addKeyValue(kProcessName, FWConfiguration(a->m_processName));
        conf.addKeyValue(kFilterExpression, FWConfiguration(a->m_filterExpression));
        iTo.addKeyValue(a->GetName(), conf, true);
    }
}

void FWAssociationManager::setFrom(const FWConfiguration& iFrom) {

  const FWConfiguration::KeyValues* keyValues = iFrom.keyValues();

  if (keyValues == nullptr)
    return;

  for (FWConfiguration::KeyValues::const_iterator it = keyValues->begin(); it != keyValues->end(); ++it) {
    const std::string& name = it->first;
    const FWConfiguration& conf = it->second;
    const FWConfiguration::KeyValues* keyValues = conf.keyValues();
    assert(nullptr != keyValues);
    const std::string& type = (*keyValues)[0].second.value();
    const std::string& moduleLabel = (*keyValues)[1].second.value();
    const std::string& productInstanceLabel = (*keyValues)[2].second.value();
    const std::string& processName = (*keyValues)[3].second.value();
    const std::string& filterExpression = (*keyValues)[4].second.value();

    auto a = new FWEveAssociation(name,
                                  TClass::GetClass(type.c_str()),
                                  moduleLabel,
                                  productInstanceLabel,
                                  processName,
                                  filterExpression);
    
    printf("add ass %s \n", name.c_str());
    m_scene->AddElement(a);
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
