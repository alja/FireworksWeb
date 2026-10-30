
#ifndef FireworksWeb_Core_GeoOverlapTableView_h
#define FireworksWeb_Core_GeoOverlapTableView_h

#include "FireworksWeb/Core/interface/FWEveView.h"

namespace ROOT {
namespace Experimental{
class REveGeoOverlapTable;
}}

class TGeoManager;

class FWWebGeoOverlapTableView : public FWEveView
{
public:
    FWWebGeoOverlapTableView(std::string type);

private:
   TGeoManager* m_geoManager{nullptr};
   ROOT::Experimental::REveGeoOverlapTable* m_table{nullptr};
};

#endif
