#ifndef FireworksWeb_Core_FWAssociationProxyBase_h
#define FireworksWeb_Core_FWAssociationProxyBase_h

#include <set>
#include <string>

class FWEveAssociation;

class FWAssociationProxyBase
{
public:
   FWAssociationProxyBase(){};
   virtual ~FWAssociationProxyBase(){};

   virtual const std::string& associatable() const = 0;
   virtual const std::string& associated() const = 0;

   virtual void getIndices(std::set<int>& in, std::set<int>& out) const = 0;

   void setEveObj(FWEveAssociation* e) { m_eveObj = e; }
   FWEveAssociation* getEveObj() const { return m_eveObj; }
   
private:
   //FWAssociationProxyBase(const FWAssociationProxyBase&); // stop default
   //const FWAssociationProxyBase& operator=(const FWAssociationProxyBase&); // stop default

   FWEveAssociation* m_eveObj{nullptr};
};

#endif
