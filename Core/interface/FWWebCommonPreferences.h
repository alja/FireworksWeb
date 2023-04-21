#ifndef FireworksWeb_Core_FWWebCommonPref2_h
#define FireworksWeb_Core_FWWebCommonPref2_h

#include "ROOT/REveElement.hxx"

class FW2CommonPref : public ROOT::Experimental::REveElement
{
    private:
    bool m_blackBg{false};
    public:
    FW2CommonPref();

   void setBackground(bool);  
   int WriteCoreJson(nlohmann::json &j, int rnr_offset) override;

};

#endif
