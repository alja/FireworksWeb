#ifndef FireworksWeb_Core_FWEveAssociation_h
#define FireworksWeb_Core_FWEveAssociation_h

#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/FWLite/interface/Event.h"
#include "DataFormats/FWLite/interface/Handle.h"

#include "ROOT/REveElement.hxx"
#include "FireworksWeb/Core/interface/FWGenericHandle.h"
#include "FireworksWeb/Core/interface/Context.h"

class FWEveAssociation : public ROOT::Experimental::REveElement
{
  friend class FWAssociationManager;

public:
  FWEveAssociation(const std::string &iName,
                   const TClass *iClass,
                   const std::string &iModuleLabel = std::string(),
                   const std::string &iProductInstanceLabel = std::string(),
                   const std::string &iProcessName = std::string(),
                   const std::string &iFilterExpression = std::string());

  ~FWEveAssociation() override {}

  void SetFilterExpr(const char *);

  void *data();

  bool filterPass(float);
  bool filterPass(std::pair<float, float>);

protected:
  const TClass *m_type;

  std::string m_moduleLabel;
  std::string m_productInstanceLabel;
  std::string m_processName;
  std::string m_filterExpression;

  std::string m_qualityType;

private:
  int WriteCoreJson(nlohmann::json &j, int rnr_offset) override;

  void initFoo1();
  void initFoo2();

  std::function<bool(float)> m_filterFoo1;
  std::function<bool(std::pair<float, float>&)> m_filterFoo2;
};

#endif
