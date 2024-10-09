
#ifndef FireworksWeb_Core_TriggerTable_h
#define FireworksWeb_Core_TriggerTable_h

#include <ROOT/REveElement.hxx>

namespace fwlite
{
    class Event;
}

class FWWebTriggerTable : public ROOT::Experimental::REveElement
{
public:
    FWWebTriggerTable(const fwlite::Event*);
    void readTriggerData();
    int WriteCoreJson(nlohmann::json &j, int rnr_offset) override;

private:
    fwlite::Event* m_event{nullptr};
 
    typedef std::unordered_map<std::string, double> acceptmap_t;

    acceptmap_t m_averageAccept;

    void fillAverageAcceptFractions();
};

#endif
