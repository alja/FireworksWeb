#ifndef FireworksWeb_Core_Context_h
#define FireworksWeb_Core_Context_h

namespace ROOT {
namespace Experimental {
class REveTrackPropagator;
class REveCaloDataHist;
}
}

class FWGeometry;
class FWMagField;
class FWBeamSpot;

namespace fireworks {
class Context {

public:
   Context();
   virtual ~Context();

   void  setGeom(const FWGeometry* x) { m_geom = x; }

   // ---------- const member functions ---------------------

   ROOT::Experimental::REveTrackPropagator* getTrackPropagator()        const { return m_propagator;        }
   ROOT::Experimental::REveTrackPropagator* getTrackerTrackPropagator() const { return m_trackerPropagator; }
   ROOT::Experimental::REveTrackPropagator* getMuonTrackPropagator()    const { return m_muonPropagator;    }

   FWMagField*          getField()             const { return m_magField; }
   FWBeamSpot*          getBeamSpot()          const { return m_beamSpot; }

   const  FWGeometry* getGeom()  const { return m_geom; }
   ROOT::Experimental::REveCaloDataHist* getCaloData() const { return m_caloData; }

   float getMaxEnergyInEvent(bool isEt) const;
   void  voteMaxEtAndEnergy(float Et, float energy) const;

   void  resetMaxEtAndEnergy() const;
   // ---------- member functions ---------------------------

   void initEveElements();
   //void deleteEveElements();


   // ---------- static member  ---------------------------

   static Context* getInstance();

   static float  caloR1(bool offset = true);
   static float  caloR2(bool offset = true);
   static float  caloZ1(bool offset = true);
   static float  caloZ2(bool offset = true);

   static float  caloTransEta();
   static float  caloTransAngle();
   static double caloMaxEta();

   static float  energyScale();

private:
   Context(const Context&) = delete; // stop default
   const Context& operator=(const Context&) = delete; // stop default

   // ---------- member data --------------------------------
   const FWGeometry     *m_geom;

   ROOT::Experimental::REveTrackPropagator  *m_propagator;
   ROOT::Experimental::REveTrackPropagator  *m_trackerPropagator;
   ROOT::Experimental::REveTrackPropagator  *m_muonPropagator;


   ROOT::Experimental::REveCaloDataHist          *m_caloData{nullptr};

   FWMagField           *m_magField;
   FWBeamSpot           *m_beamSpot;

   mutable float                 m_maxEt;
   mutable float                 m_maxEnergy;

   mutable bool          m_hidePFBuilders;

   static Context* s_fwContext;

   // calo data
   static const float s_caloTransEta;
   static const float s_caloTransAngle;
   // simplified
   static const float s_caloR;
   static const float s_caloZ;

   // proxy-builder offsets
   static const float s_caloOffR;
   static const float s_caloOffZ;
};
}
#endif
