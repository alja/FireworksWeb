#ifndef FireworksWeb_Core_FWViewManager_h
#define FireworksWeb_Core_FWViewManager_h

#include "FireworksWeb/Core/interface/FWConfigurable.h"
#include "FireworksWeb/Core/interface/fwLog.h"

class FWEveView;
class FW2Main;

class FWViewManager : public FWConfigurable 
{
public:
  friend class FW2EveManager;

  FWViewManager();
  ~FWViewManager() override;

  // add a view to be managed
  void addView(FWEveView* view);

  // clear all view setups
  void clearViews();

  // FWConfigurable interface
  void addTo(FWConfiguration& oConfig) const override;
  void setFrom(const FWConfiguration& iConfig) override;

protected:
  std::vector<FWEveView*> m_views;
};

#endif
