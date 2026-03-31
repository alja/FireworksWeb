
#ifndef FireworksWeb_Core_GeoTableView_h
#define FireworksWeb_Core_GeoTableView_h

// #include <ROOT/REveElement.hxx>
#include "FireworksWeb/Core/interface/FWEveView.h"


namespace ROOT {
namespace Experimental{
class REveGeoTopNode;
}}

class FWWebGeoTableView : public FWEveView
{
public:
    FWWebGeoTableView(std::string type);
    // int WriteCoreJson(nlohmann::json &j, int rnr_offset) override;

private:
   ROOT::Experimental::REveGeoTopNode* m_topNode{nullptr};
};

#endif
