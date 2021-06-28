#ifndef FireworksWeb_Core_FWRPZView_h
#define FireworksWeb_Core_FWRPZView_h

#include "FireworksWeb/Core/interface/FWEveView.h"

//#include "FireworksWeb/Core/interface/FWDoubleParameter.h"
//#include "FireworksWeb/Core/interface/FWBoolParameter.h"

namespace ROOT {
    namespace Experimental {
        class REveProjectionManager;
        class REveElement;
    }
}

class FWRPZView : public FWEveView
{
public:
    FWRPZView(std::string vtype);
    virtual ~FWRPZView();

    void eventBegin() override;
    void eventEnd() override;
    void importElements( ROOT::Experimental::REveElement* iProjectableChild, float layer, ROOT::Experimental::REveElement* iProjectedParent = nullptr);
    //void setupEventCenter() override;
protected:


  // parameters

  ROOT::Experimental::REveProjectionManager* m_projMgr{nullptr};

private:
  FWRPZView(const FWRPZView&) = delete;                   // stop default
  const FWRPZView& operator=(const FWRPZView&) = delete;  // stop default

};

#endif
