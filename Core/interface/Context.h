#ifndef FireworksWeb_Core_Context_h
#define FireworksWeb_Core_Context_h

namespace ROOT {
namespace Experimental {
class REveTrackPropagator;
class REveCaloDataHist;
class REveDataCollection;
}
}

namespace fwlite {
   class Event;
}

class FWGeometry;
class FWMagField;
class FWBeamSpot;
class FWViewEnergyScale;
class FW2Main;
class FWWebEventItem;
class FWColorManager;

namespace fireworks {
class Context {

public:
   Context(const FW2Main*);
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

   bool isOpendata() const;  // AMT temprary solution needed in eve view restore until window mamanger gets itnroduced

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

   static float mtdEtlR1();
   static float mtdEtlR2();
   static float mtdEtlZ1(const unsigned int& disk_number = 1);
   static float mtdEtlZ2(const unsigned int& disk_number = 1);

   FWViewEnergyScale* energyScale() { return m_energyScale; }
   const fwlite::Event* getCurrentEvent() const;

   FWColorManager* colorManager() const { return m_colorManager; }

   // markers
   static const float s_markerScale; // temporary factor unil issue with marker scaling is resolved
   
private:
   Context(const Context&) = delete; // stop default
   const Context& operator=(const Context&) = delete; // stop default

   // ---------- member data --------------------------------
   const FW2Main        *m_main{nullptr};
   const FWGeometry     *m_geom{nullptr};

   FWColorManager       *m_colorManager{nullptr};

   ROOT::Experimental::REveTrackPropagator  *m_propagator{nullptr};
   ROOT::Experimental::REveTrackPropagator  *m_trackerPropagator{nullptr};
   ROOT::Experimental::REveTrackPropagator  *m_muonPropagator{nullptr};


   ROOT::Experimental::REveCaloDataHist          *m_caloData{nullptr};
   
   FWViewEnergyScale    *m_energyScale{nullptr};

   FWMagField           *m_magField{nullptr};
   FWBeamSpot           *m_beamSpot{nullptr};

   mutable float                 m_maxEt{1.f};
   mutable float                 m_maxEnergy{1.f};

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

   // mtd data
   static const float s_mtdEtlR1;
   static const float s_mtdEtlR2;
   static const float s_mtdEtlZ1;
   static const float s_mtdEtlZ2;
   static const float s_mtdEtlOffZ; 
};
}
#endif
