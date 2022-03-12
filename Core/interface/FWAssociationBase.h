#ifndef FireworksWeb_Core_FWAssociationBase_h
#define FireworksWeb_Core_FWAssociationBase_h

#include <set>
#include <string>

class FWAssociationBase
{
public:
   FWAssociationBase(){};
   virtual ~FWAssociationBase(){};

   virtual const std::string& associatable() const = 0;
   virtual const std::string& associated() const = 0;

   virtual void getIndices(std::set<int>& in, std::set<int>& out) const = 0;  
   
private:
   //FWAssociationBase(const FWAssociationBase&); // stop default
   //const FWAssociationBase& operator=(const FWAssociationBase&); // stop default
};

#endif
