#ifndef FireworksWeb_Core_FWGUIEventFilter_h
#define FireworksWeb_Core_FWGUIEventFilter_h

#include "ROOT/REveElement.hxx"

class CmsShowNavigator;
class FWGUIEventSelector;

class FWGUIEventFilter : public ROOT::Experimental::REveElement
{
public:
    FWGUIEventFilter(CmsShowNavigator *);
    ~FWGUIEventFilter() override;
   // std::list<FWGUIEventSelector *> &guiSelectors() { return m_guiSelectors; }

    int WriteCoreJson(nlohmann::json &j, int rnr_offset) override;

private:
    // std::list<FWGUIEventSelector *> m_guiSelectors;
    CmsShowNavigator *m_navigator{nullptr};
};

#endif
