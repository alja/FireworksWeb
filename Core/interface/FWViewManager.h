#ifndef FireworksWeb_Core_FWViewManager_h
#define FireworksWeb_Core_FWViewManager_h

#include "FireworksWeb/Core/interface/FWConfigurable.h"

class FWEveView;
class FW2Main;

class FWViewManager : public FWConfigurable 
{
public:
  friend class FW2EveManager;

  FWViewManager();
  ~FWViewManager() override;

  void addTo(FWConfiguration&) const override;
  void setFrom(const FWConfiguration&) override;

protected:
  std::vector<FWEveView*> m_views;
};

#endif
