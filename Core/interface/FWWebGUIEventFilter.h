#ifndef FireworksWeb_Core_FWWebGUIEventFilter_h
#define FireworksWeb_Core_FWWebGUIEventFilter_h

#include "ROOT/REveElement.hxx"

class CmsShowNavigator;
class FWGUIEventSelector;



class FWWebGUIEventFilter : public ROOT::Experimental::REveElement
{
public:
    FWWebGUIEventFilter(CmsShowNavigator *);
    ~FWWebGUIEventFilter() override;
   // std::list<FWGUIEventSelector *> &guiSelectors() { return m_guiSelectors; }

    int WriteCoreJson(nlohmann::json &j, int rnr_offset) override;
    void SetFilterEnabled(bool);
    void PublishFilters(const char *arg);

private:
    // std::list<FWGUIEventSelector *> m_guiSelectors;
    CmsShowNavigator *m_navigator{nullptr};
   // std::list<FWGUIEventSelector*> m_guiSelectors;
};

#endif
