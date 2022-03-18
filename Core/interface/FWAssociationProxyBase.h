#ifndef FireworksWeb_Core_FWAssociationProxyBase_h
#define FireworksWeb_Core_FWAssociationProxyBase_h

#include <set>
#include <string>

class FWAssociationProxyBase
{
public:
   FWAssociationProxyBase(){};
   virtual ~FWAssociationProxyBase(){};

   virtual const std::string& associatable() const = 0;
   virtual const std::string& associated() const = 0;

   virtual void getIndices(std::set<int>& in, std::set<int>& out) const = 0;  
   
private:
   //FWAssociationProxyBase(const FWAssociationProxyBase&); // stop default
   //const FWAssociationProxyBase& operator=(const FWAssociationProxyBase&); // stop default
};

#endif
