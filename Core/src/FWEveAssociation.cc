#include "FireworksWeb/Core/interface/FWEveAssociation.h"

#include "TROOT.h"
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
    SetTitle(iClass->GetName());
}

void FWEveAssociation::SetFilterExpr(const char* x)
{
  m_filterExpression = x;
  printf("Set filter expression .... %s \n", x);
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

void FWEveAssociation::initFoo1()
{
  std::stringstream s;
  s << "*((std::function<bool(float)>*)" << std::hex << std::showbase
    << (size_t)&m_filterFoo1 << ") = [](float p){ float &i= p; return (" << m_filterExpression.c_str() << "); };";

  std::cout << "\n\ninit_1    " << s.str() << "\n";
  try
  {
    gROOT->ProcessLine(s.str().c_str());
  }
  catch (const std::exception &exc)
  {
    std::cout << "Errr" << exc.what() << "\n";
  }
}

void FWEveAssociation::initFoo2()
{
  std::stringstream s;
  s << "*((std::function<bool( std::pair<float, float> )>*)" << std::hex << std::showbase
    << (size_t)&m_filterFoo2 << ") = [](std::pair<float, float> p){ std::pair<float, float> &i= p; return (" << m_filterExpression.c_str() << "); };";

  std::cout << "\n\ninit_2    " << s.str() << "\n";
  try
  {
    gROOT->ProcessLine(s.str().c_str());
  }
  catch (const std::exception &exc)
  {
    std::cout << "Errr" << exc.what() << "\n";
  }
}


bool FWEveAssociation::filterPass(float p)
{
  if (m_filterExpression.empty())
    return true;

  if (!m_filterFoo1)
    initFoo1();

  return m_filterFoo1(p);
}

bool FWEveAssociation::filterPass(std::pair<float, float> p)
{
  if (m_filterExpression.empty())
    return true;

  if (!m_filterFoo2)
    initFoo2();

  bool res = m_filterFoo2(p);
  printf("%d => %s (%f, %f) \n", res, m_filterExpression.c_str(), p.first, p.second);

  return res;
}
