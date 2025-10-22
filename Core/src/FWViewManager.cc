#include "ROOT/REveScene.hxx"
#include "ROOT/REveManager.hxx"

#include "FireworksWeb/Core/interface/FWConfiguration.h"
#include "FireworksWeb/Core/interface/FWViewManager.h"
#include "FireworksWeb/Core/interface/FWEveView.h"

#include <iostream>
// using namespace ROOT::Experimental;

FWViewManager::FWViewManager() {

}

FWViewManager::~FWViewManager() {}


//______________________________________________________________________________
void FWViewManager::addTo(FWConfiguration &iTo) const
{
    for (auto* view : m_views)
    {
        FWConfiguration conf;
        conf.addKeyValue("ViewType", FWConfiguration(view->viewType()));
/*
        FWConfiguration viewSettingConf;
        view->addTo(viewSettingConf);
        conf.addKeyValue("ViewController", viewSettingConf);

        iTo.addKeyValue("View", conf, true);
        */
    }
}

//__________________________________________________________________________________
void FWViewManager::setFrom(const FWConfiguration &iFrom)
{

    const FWConfiguration::KeyValues *keyValues = iFrom.keyValues();

    if (keyValues == nullptr)
        return;

    for (FWConfiguration::KeyValues::const_iterator it = keyValues->begin(); it != keyValues->end(); ++it)
    {
        const std::string &name = it->first;
        // std::cout << "WViewManager::setFrom() got entry for " << name << std::endl;
    }
}
