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

void FWViewManager::addView(FWEveView* view) {
   if (view) {
      m_views.push_back(view);
   }
}

void FWViewManager::clearViews() {
   m_views.clear();
}

//______________________________________________________________________________
void FWViewManager::addTo(FWConfiguration &iTo) const
{
    for (auto* view : m_views)
    {
        if (!view) continue;

        FWConfiguration viewConfig;

        // let the view save its own settings
        view->addTo(viewConfig);

        //use the view type as the key
        std::string viewType = view->viewType();

        // add this view's configuration under its type name
        iTo.addKeyValue(viewType, viewConfig, true);

        fwLog(fwlog::kDebug) << "FWViewManager::addTo() saved view: " << viewType << std::endl;
        // conf.addKeyValue("ViewType", FWConfiguration(view->viewType()));
    }
}

//__________________________________________________________________________________
void FWViewManager::setFrom(const FWConfiguration &iFrom)
{

    const FWConfiguration::KeyValues *keyValues = iFrom.keyValues();

    if (keyValues == nullptr) {
        fwLog(fwlog::kWarning) << "FWViewManager::setFrom() no view configuration found\n";
        return;
    }

    for (const auto& kv : *keyValues) {
        const std::string& viewType = kv.first;
        const FWConfiguration& viewConfig = kv.second;
        
        fwLog(fwlog::kDebug) << "FWViewManager::setFrom() loading view: " << viewType << std::endl;
        
        // Find the view with matching type
        bool found = false;
        for (auto* view : m_views) {
            if (view && view->viewType() == viewType) {
                view->setFrom(viewConfig);
                found = true;
                fwLog(fwlog::kInfo) << "Restored configuration for view: " << viewType << std::endl;
                break;
            }
        }
        
        if (!found) {
            fwLog(fwlog::kWarning) << "No view found for configuration: " << viewType << std::endl;
        }
    }
}
