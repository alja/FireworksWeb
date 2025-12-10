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
class FWWebTriggerTable;
class FWEventAnnotation;

// namespace REX = ROOT::Experimantal;

class FWEveView : public ROOT::Experimental::REveElement,
                  public FWConfigurableParameterizable {
public:
  FWEveView(std::string vtype);
  virtual ~FWEveView();

  // FWConfigurable interface
  virtual void addTo(FWConfiguration& oConfig) const;
  virtual void setFrom(const FWConfiguration& iConfig);

  virtual void eventEnd();
  virtual void eventBegin();

  virtual void bgChanged(bool is_dark){m_blackBackground = is_dark;}

  ROOT::Experimental::REveViewer* viewer() {return m_viewer;}
  ROOT::Experimental::REveViewer* viewer() const {return m_viewer;}

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

  void setBlackBackground(bool black);
  bool getBlackBackground() const { return m_blackBackground; }
  void setAxesType(int type);
  int getAxesType() const { return m_axesType; }

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
  bool m_blackBackground;
  int m_axesType;

};

//==============================================================================
//==============================================================================

class FWWebTableView : public FWEveView
{
public:
  FWWebTableView(std::string vtype);
  void  importContext(ROOT::Experimental::REveViewContext*) override;
};

//==============================================================================
class FWWebTriggerTableView : public FWEveView
{
  FWWebTriggerTable* m_triggerTable{nullptr};
public:
  FWWebTriggerTableView(std::string vtype);
  void eventEnd() override;
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
