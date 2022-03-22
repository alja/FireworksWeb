#ifndef FireworksWeb_Core_FWEveAssociation_h
#define FireworksWeb_Core_FWEveAssociation_h

#include "ROOT/REveElement.hxx"

class FWEveAssociation : public ROOT::Experimental::REveElement {
public:
  FWEveAssociation() {}
  ~FWEveAssociation() override {}

  void SetFilterExpr(const char*);

private:
  std::string fFilterExpr;

};

#endif
