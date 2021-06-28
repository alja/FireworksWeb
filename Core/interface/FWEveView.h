#include <string>


#ifndef FireworksWeb_Core_FWEveView_h
#define FireworksWeb_Core_FWEveView_h

namespace ROOT {
    namespace Experimental {
        class REveViewer;
        class REveScene;
    }
}

namespace fireworks
{
    class Context;
}

// namespace REX = ROOT::Experimantal;

class FWEveView {
public:
  FWEveView(std::string vtype);
  virtual ~FWEveView();

  // void setFrom(const FWConfiguration&) override;
  // void addTo(FWConfiguration&) const override;

  virtual void eventEnd();
  virtual void eventBegin();

  ROOT::Experimental::REveViewer* viewer();

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

  const std::string& viewType()const {return m_viewType;}
protected:

  /*
  virtual ROOT::Experimental::REveCaloViz* geROOT::Experimental::REveCalo() const { return nullptr; }
  void addToOrthoCamera(TGLOrthoCamera*, FWConfiguration&) const;
  void setFromOrthoCamera(TGLOrthoCamera*, const FWConfiguration&);
  void addToPerspectiveCamera(TGLPerspectiveCamera*, const std::string&, FWConfiguration&) const;
  void setFromPerspectiveCamera(TGLPerspectiveCamera*, const std::string&, const FWConfiguration&);
*/

private:
  FWEveView(const FWEveView&) = delete;                   // stop default
  const FWEveView& operator=(const FWEveView&) = delete;  // stop default

  // ---------- member data --------------------------------

  ROOT::Experimental::REveViewer* m_viewer{nullptr};
  ROOT::Experimental::REveScene* m_eventScene{nullptr};
  //ROOT::Experimental::REveElement* m_ownedProducts;
  ROOT::Experimental::REveScene* m_geoScene{nullptr};
  std::string m_viewType;

};

/*
class FW3DView : public FWEveView
{
public:
  FWEveView(std::string vtype);
  ~FWEveView() override;
};

class FWTableView : public FWEveView
{

};
*/
#endif
