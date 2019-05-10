#ifndef Fireworks2_Muons_FWMuonBuilder_h
#define Fireworks2_Muons_FWMuonBuilder_h

namespace reco {
   class Muon;
}

namespace ROOT {
namespace Experimental {   
class REveElementList;
class REveTrackPropagator;
class REveDataProxyBuilderBase;
}
}

class FWEventItem;
class FWMagField;
class FWProxyBuilderBase;

class FWMuonBuilder
{

public:
   FWMuonBuilder();
   virtual ~FWMuonBuilder();

   // ---------- const member functions ---------------------

   // ---------- static member functions --------------------

   // ---------- member functions ---------------------------
   void buildMuon(ROOT::Experimental::REveDataProxyBuilderBase*,
                  const reco::Muon* muon,
                  ROOT::Experimental::REveElement* tList,
                  bool showEndcap,
                  bool onlyTracks = false);

   void setLineWidth(int w) {m_lineWidth = w;}
private:
   FWMuonBuilder(const FWMuonBuilder&) = delete;    // stop default

   const FWMuonBuilder& operator=(const FWMuonBuilder&) = delete;    // stop default

   void calculateField(const reco::Muon& iData, FWMagField* field);

   // ---------- member data --------------------------------
   int m_lineWidth;
};

#endif
