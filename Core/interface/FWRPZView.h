#ifndef FireworksWeb_Core_FWRPZView_h
#define FireworksWeb_Core_FWRPZView_h

#include "FireworksWeb/Core/interface/FWEveView.h"


namespace ROOT {
    namespace Experimental {
        class REveProjectionManager;
        class REveElement;
        class REveCalo2D;
    }
}
class FWRPZViewGeometry;

class FWRPZView : public FW3DView
{
public:
    FWRPZView(std::string vtype);
    virtual ~FWRPZView() override;

    void eventBegin() override;
    void eventEnd() override;
    void importElements(ROOT::Experimental::REveElement *iProjectableChild, float layer, ROOT::Experimental::REveElement *iProjectedParent);
    void importContext(ROOT::Experimental::REveViewContext *m_viewContext) override;
    virtual ROOT::Experimental::REveCaloViz *getEveCalo() const override;

protected:
    ROOT::Experimental::REveCalo2D *m_calo{nullptr};
    ROOT::Experimental::REveProjectionManager *m_projMgr{nullptr};

private:
    FWRPZView(const FWRPZView &) = delete;                  // stop default
    const FWRPZView &operator=(const FWRPZView &) = delete; // stop default
    FWRPZViewGeometry* m_geometryList{nullptr};
};

#endif
