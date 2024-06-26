
#include "ROOT/REveDataCollection.hxx"
#include "ROOT/REveScene.hxx"
#include "ROOT/REveManager.hxx"

#include "FireworksWeb/Core/interface/FWAssociationProxyBase.h"
#include "FireworksWeb/Core/interface/FWAssociationFactory.h"
#include "FireworksWeb/Core/interface/FWConfiguration.h"
#include "FireworksWeb/Core/interface/FWAssociationManager.h"
#include "FireworksWeb/Core/interface/FWEveAssociation.h"
#include "FireworksWeb/Core/interface/FWPhysicsObjectDesc.h"

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



//______________________________________________________________________________
void FWAssociationManager::addAssociationInternal(const std::string &name, const std::string &type,
                                          const std::string &moduleLabel, const std::string &productInstanceLabel,
                                          const std::string &processName, const std::string &filterExpression)
{
    TClass *c = TClass::GetClass(type.c_str());
    auto a = new FWEveAssociation(name,
                                  c,
                                  moduleLabel,
                                  productInstanceLabel,
                                  processName,
                                  filterExpression);
    a->changed_.connect(std::bind(&FWAssociationManager::filterChanged, this));
    m_scene->AddElement(a);

    std::cout << "FWAssociationManager::addAssociationInternal 1\n";
    edm::TypeWithDict modelType(*(a->m_type->GetTypeInfo()));
    std::string atn = modelType.typeInfo().name();

    std::vector<edmplugin::PluginInfo> ac = edmplugin::PluginManager::get()->categoryToInfos().find(FWAssociationFactory::get()->category())->second;
    for (auto &i : ac) // loop plugins
    {
        std::string pnh = i.name_;
        std::string pn = pnh.substr(0, pnh.find_first_of('@'));

        if (atn == pn)
        {
            m_associations.push_back(FWAssociationFactory::get()->create(pnh));
            //std::cout << "FWAssociationManager::addAssociationInternal associatable .... " << m_associations.back()->associatable() << " associated " << m_associations.back()->associated() << std::endl;
            m_associations.back()->setEveObj(a);
            break;
        }
    }
}

//______________________________________________________________________________
void FWAssociationManager::addAssociation(FWPhysicsObjectDesc& d)
{
    std::string name = d.purpose() + "_" + "Association";
    addAssociationInternal(name, d.type()->GetName(),
                   d.moduleLabel(), d.productInstanceLabel(),
                   d.processName(), d.filterExpression());
}

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

//__________________________________________________________________________________
void FWAssociationManager::setFrom(const FWConfiguration &iFrom)
{

    const FWConfiguration::KeyValues *keyValues = iFrom.keyValues();

    if (keyValues == nullptr)
        return;

    for (FWConfiguration::KeyValues::const_iterator it = keyValues->begin(); it != keyValues->end(); ++it)
    {
        const std::string &name = it->first;
        const FWConfiguration &conf = it->second;
        const FWConfiguration::KeyValues *keyValues = conf.keyValues();
        assert(nullptr != keyValues);
        const std::string &type = (*keyValues)[0].second.value();
        const std::string &moduleLabel = (*keyValues)[1].second.value();
        const std::string &productInstanceLabel = (*keyValues)[2].second.value();
        const std::string &processName = (*keyValues)[3].second.value();
        const std::string &filterExpression = (*keyValues)[4].second.value();

        addAssociationInternal(name, type,
                               moduleLabel, productInstanceLabel,
                               processName, filterExpression);
    }
}

//__________________________________________________________________________________
void FWAssociationManager::filterChanged()
{
    printf("\n\n\nFWAssociationManager::filterChanged\n");

    // AMT TODO: make this work for multiple selection
    gEve->GetSelection()->NewElementPicked(0, false, false);
    gEve->GetSelection()->NewElementPicked(m_selectionDeviator->m_selected->GetElementId(), false, true, m_selectionDeviator->m_selected->RefSelectedSet());
}

//__________________________________________________________________________________
void FWAssociationManager::refAssociationTypes(std::vector<std::string> &in) const
{
    std::vector<edmplugin::PluginInfo> ac = edmplugin::PluginManager::get()->categoryToInfos().find(FWAssociationFactory::get()->category())->second;
    {
        // std::cout << "\n============= eve ass Look match for tpe " << typeName << "\n";
        for (auto &i : ac) // loop plugins
        {
            std::string pnh = i.name_;
          //  std::string pn = pnh.substr(0, pnh.find_first_of('@'));
            in.push_back(pnh);
        }
    }
}

//__________________________________________________________________________________
void FWAssociationManager::FWSelectionDeviator::SelectAssociated(REveSelection *selection, REveDataItemList *colItems)
{
    REveDataCollection *ac = static_cast<REveDataCollection *>(colItems->GetMother());

    REveElement *collectionList = ac->GetMother();
    std::string itemClass = ac->GetItemClass()->GetName();
    for (auto &ap : m_mng->m_associations)
    {
        if (ap->associatable() == itemClass && ap->getEveObj()->GetRnrSelf())
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
            m_selected = colItems;

            if (selection == gEve->GetSelection())
                SelectAssociated(selection, colItems);

            return true;
        }
    }
    return false;
}
