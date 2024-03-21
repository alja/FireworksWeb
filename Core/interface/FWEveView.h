#include <string>


#ifndef FireworksWeb_Core_FWEveView_h
#define FireworksWeb_Core_FWEveView_h

#include "ROOT/REveElement.hxx"
#include "FireworksWeb/Core/interface/FWConfigurableParameterizable.h"
#include "FireworksWeb/Core/interface/FWBoolParameter.h"
#include "FireworksWeb/Core/interface/FWLongParameter.h"
#include "FireworksWeb/Core/interface/FWEnumParameter.h"

namespace ROOT {
    namespace Experimental {
        class REveViewer;
        class REveScene;
        class REveViewContext;
        class REveCaloViz;
        class REveCalo3D;
        class REveCaloLego;
        class REvePointSet;
        class REveBoxSet;
    }
}

namespace fireworks
{
    class Context;
}

class TPad;
class FWTriggerTable;

// namespace REX = ROOT::Experimantal;

class FWEveView : public ROOT::Experimental::REveElement,
                  public FWConfigurableParameterizable {
public:
  FWEveView(std::string vtype);
  virtual ~FWEveView();

  // void setFrom(const FWConfiguration&) override;
  // void addTo(FWConfiguration&) const override;

  virtual void eventEnd();
  virtual void eventBegin();

  ROOT::Experimental::REveViewer* viewer() {return m_viewer;}

  ROOT::Experimental::REveScene* eventScene() { return m_eventScene; }
  ROOT::Experimental::REveScene* geoScene() { return m_geoScene; }

  fireworks::Context* context();
  /*
  virtual void useGlobalEnergyScaleChanged();
  virtual bool isEnergyScaleGlobal() const;
  virtual void setupEnergyScale();
  virtual void setupEventCenter();
  virtual void voteCaloMaxVal();
*/

  std::string viewType()const {return m_viewType;}

  virtual ROOT::Experimental::REveCaloViz* getEveCalo() const { return nullptr; }
  virtual void importContext(ROOT::Experimental::REveViewContext*) {};
  void setupEnergyScale();

protected:

  /*
  virtual ROOT::Experimental::REveCaloViz* geROOT::Experimental::REveCalo() const { return nullptr; }
  void addToOrthoCamera(TGLOrthoCamera*, FWConfiguration&) const;
  void setFromOrthoCamera(TGLOrthoCamera*, const FWConfiguration&);
  void addToPerspectiveCamera(TGLPerspectiveCamera*, const std::string&, FWConfiguration&) const;
  void setFromPerspectiveCamera(TGLPerspectiveCamera*, const std::string&, const FWConfiguration&);
*/
  ROOT::Experimental::REveViewer* m_viewer{nullptr};
  ROOT::Experimental::REveScene* m_eventScene{nullptr};
  ROOT::Experimental::REveScene* m_geoScene{nullptr};
  std::string m_viewType;

private:
  FWEveView(const FWEveView&) = delete;                   // stop default
  const FWEveView& operator=(const FWEveView&) = delete;  // stop default

  void voteCaloMaxVal();
};

//==============================================================================
//==============================================================================

class FWTableView : public FWEveView
{
public:
  FWTableView(std::string vtype);
  void  importContext(ROOT::Experimental::REveViewContext*) override;
};

//==============================================================================
class FWTriggerTableView : public FWEveView
{
  FWTriggerTable* m_triggerTable{nullptr};
public:
  FWTriggerTableView(std::string vtype);
  void eventEnd() override;
};


//==============================================================================
//==============================================================================
class FW3DView : public FWEveView
{
private:
  ROOT::Experimental::REveCalo3D* m_calo3d{nullptr};

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

  ROOT::Experimental::REveBoxSet* m_ecalBarrel{nullptr};
public:
  FW3DView(std::string vtype);
  ~FW3DView() override;

  // void eventEnd() override;
  void  importContext(ROOT::Experimental::REveViewContext*) override;
  virtual ROOT::Experimental::REveCaloViz* getEveCalo() const override;
  int WriteCoreJson(nlohmann::json &j, int rnr_offset) override;


  void showMuonBarrel(bool x) {m_showMuonBarrel.set(x); StampObjProps();}
  void showMuonEndcap(bool x) {m_showMuonEndcap.set(x); StampObjProps();}
  void showPixelBarrel(bool x) {m_showPixelBarrel.set(x); StampObjProps();}
  void showPixelEndcap(bool x) {m_showPixelEndcap.set(x); StampObjProps();}
  void showTrackerBarrel(bool x) {m_showTrackerBarrel.set(x); StampObjProps();}
  void showTrackerEndcap(bool x) {m_showTrackerEndcap.set(x); StampObjProps();}
  void showEcalBarrel(bool x);// {m_showEcalBarrel.set(x); StampObjProps();}
};

//==============================================================================
//==============================================================================

class FWLegoView : public FWEveView
{
private:
  ROOT::Experimental::REvePointSet* m_lego{nullptr};
  TPad* m_pad{nullptr};

public:
  FWLegoView(std::string vtype = "Lego");
  ~FWLegoView() override;

  void eventEnd() override;
  void importContext(ROOT::Experimental::REveViewContext*) override;
  //virtual ROOT::Experimental::REveCaloViz* getEveCalo() const override;
};



#endif
