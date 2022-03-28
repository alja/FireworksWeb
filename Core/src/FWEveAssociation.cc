#include "FireworksWeb/Core/interface/FWEveAssociation.h"

#include "nlohmann/json.hpp"

FWEveAssociation::FWEveAssociation(const std::string& iName,
                                         const TClass* iClass,
                                         const std::string& iModuleLabel,
                                         const std::string& iProductInstanceLabel,
                                         const std::string& iProcessName,
                                         const std::string& iFilterExpression):
   m_type(iClass),
   m_moduleLabel(iModuleLabel),
   m_productInstanceLabel(iProductInstanceLabel),
   m_processName(iProcessName),
   m_filterExpression(iFilterExpression)
{
    SetName(iName);
}

void FWEveAssociation::SetFilterExpr(const char* x)
{
  m_filterExpression = x;
}

void *FWEveAssociation::data()
{
    edm::InputTag tag(m_moduleLabel, m_productInstanceLabel, m_processName);

    edm::TypeWithDict type(*(m_type->GetTypeInfo()));
    edm::FWGenericHandle handle(type);

    const fwlite::Event *event = fireworks::Context::getInstance()->getCurrentEvent();
    event->getByLabel(tag, handle);

    // const edm::ObjectWithDict& data = *handle;
    return handle->address();
}

int FWEveAssociation::WriteCoreJson(nlohmann::json &j, int rnr_offset)
{
  using namespace nlohmann;
  int ret = REveElement::WriteCoreJson(j, rnr_offset);
  j["FilterExpr"] = m_filterExpression;

  return ret;
}
