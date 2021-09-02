// -*- C++ -*-
#ifndef FireworksWeb_Core_FWTableViewManager_h
#define FireworksWeb_Core_FWTableViewManager_h
//
// Package:     Core
// Class  :     FWTableViewManager
//
/**\class FWTableViewManager FWTableViewManager.h FireworksWeb/Core/interface/FWTableViewManager.h

   Description: Base class for a Manger for a specific type of View

   Usage:
   <usage>

*/
//
// Original Author:
//         Created:  Sat Jan  5 10:29:00 EST 2008
//
#include "FireworksWeb/Core/interface/FWConfigurable.h"
#include <ROOT/REveTableInfo.hxx>

namespace ROOT::Experimental
{
   class REveTableViewInfo;
}

class FWTableViewManager :  public FWConfigurable 
{
private:
   ROOT::Experimental::REveTableViewInfo* m_tableInfo;
   
   static const std::string kConfigTypeNames;
   static const std::string kConfigDisplayedCollection;
public:
   FWTableViewManager();
   
   ROOT::Experimental::REveTableViewInfo* getTableInfo() { return m_tableInfo; }
   const std::string&  getDisplayedCollection() const;
   void               checkExpressionsForType(TClass* itemclass);

   void                    addTo(FWConfiguration&) const override;
   void                    setFrom(const FWConfiguration&) override;
   
};
#endif
