#ifndef FireworksWeb_Core_FW3DEveView_h
#define FireworksWeb_Core_FW3DEveView_h


#include "FireworksWeb/Core/interface/FWEveView.h"

//==============================================================================
//==============================================================================
class FW3DView : public FWEveView
{
private:
  ROOT::Experimental::REveCalo3D* m_calo3d{nullptr};
  FWEventAnnotation* m_annotation{nullptr};

  // parameters
  FWBoolParameter m_showMuonBarrel;
  FWBoolParameter m_showMuonEndcap;
  FWBoolParameter m_showPixelBarrel;
  FWBoolParameter m_showPixelEndcap;
  FWBoolParameter m_showTrackerBarrel;
  FWBoolParameter m_showTrackerEndcap;/*
  FWBoolParameter m_showHGCalEE;
  FWBoolParameter m_showHGCalHSi;
  FWBoolParameter m_showHGCalHSc;
  FWBoolParameter m_showMtdBarrel;
  FWBoolParameter m_showMtdEndcap;*/
  FWBoolParameter m_showEcalBarrel;
  FWBoolParameter m_showEventLabel;

  ROOT::Experimental::REveBoxSet* m_ecalBarrel{nullptr};
public:
  FW3DView(std::string vtype);
  ~FW3DView() override;

  // void eventEnd() override;
  void  importContext(ROOT::Experimental::REveViewContext*) override;
  ROOT::Experimental::REveCaloViz* getEveCalo() const override;
  int WriteCoreJson(nlohmann::json &j, int rnr_offset) override;

  void setFrom(const FWConfiguration&) override;
  void addTo(FWConfiguration&) const override;

  void bgChanged(bool is_dark) override;

  void eventEnd() override;

  void showMuonBarrel(bool x) {m_showMuonBarrel.set(x); StampObjProps();}
  void showMuonEndcap(bool x) {m_showMuonEndcap.set(x); StampObjProps();}
  void showPixelBarrel(bool x) {m_showPixelBarrel.set(x); StampObjProps();}
  void showPixelEndcap(bool x) {m_showPixelEndcap.set(x); StampObjProps();}
  void showTrackerBarrel(bool x) {m_showTrackerBarrel.set(x); StampObjProps();}
  void showTrackerEndcap(bool x) {m_showTrackerEndcap.set(x); StampObjProps();}
  void showEcalBarrel(bool x);// {m_showEcalBarrel.set(x); StampObjProps();}

  void showEventLabel(bool);
};
#endif