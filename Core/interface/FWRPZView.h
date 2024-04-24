
#ifndef FireworksWeb_Core_FWRPZView_h
#define FireworksWeb_Core_FWRPZView_h

#include "FireworksWeb/Core/interface/FWEveView.h"

#include "FireworksWeb/Core/interface/FWDoubleParameter.h"
#include "FireworksWeb/Core/interface/FWBoolParameter.h"


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

    void showPixelBarrel(bool x) {m_showPixelBarrel.set(x); StampObjProps();}
    void showPixelEndcap(bool x) {m_showPixelEndcap.set(x); StampObjProps();}
    void showTrackerBarrel(bool x) {m_showTrackerBarrel.set(x); StampObjProps();}
    void showTrackerEndcap(bool x) {m_showTrackerEndcap.set(x); StampObjProps();}
    void showRpcEndcap(bool x) {m_showRpcEndcap.set(x); StampObjProps();}
    void showMtdEndcap(bool x) {m_showMtdEndcap.set(x); StampObjProps();}
    void showGEM(bool x) {m_showGEM.set(x); StampObjProps();}
    void showME0(bool x) {m_showME0.set(x); StampObjProps();}
    
    int WriteCoreJson(nlohmann::json &j, int rnr_offset) override;
    virtual void bgChanged() override;
protected:
    ROOT::Experimental::REveCalo2D *m_calo{nullptr};
    ROOT::Experimental::REveProjectionManager *m_projMgr{nullptr};

private:
    FWRPZView(const FWRPZView &) = delete;                  // stop default
    const FWRPZView &operator=(const FWRPZView &) = delete; // stop default
    FWRPZViewGeometry* m_geometryList{nullptr};

    FWBoolParameter m_shiftOrigin;
    FWDoubleParameter m_fishEyeDistortion;
    FWDoubleParameter m_fishEyeR;

    FWDoubleParameter m_caloDistortion;
    FWDoubleParameter m_muonDistortion;
    FWBoolParameter m_showProjectionAxes;
    FWBoolParameter m_compressMuon;

    FWBoolParameter m_showPixelBarrel;
    FWBoolParameter m_showPixelEndcap;
    FWBoolParameter m_showTrackerBarrel;
    FWBoolParameter m_showTrackerEndcap;
    FWBoolParameter m_showRpcEndcap;
    FWBoolParameter m_showGEM;
    FWBoolParameter m_showME0;
    FWBoolParameter m_showMtdBarrel;
    FWBoolParameter m_showMtdEndcap;

    void doPreScaleDistortion();
    void doFishEyeDistortion();
    void doCompression(bool);

    bool m_geoInitialized{false}; // temporary solution
    // void doShiftOriginToBeamSpot();
};

#endif
