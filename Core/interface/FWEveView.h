#include <string>


#ifndef FireworksWeb_Core_FWEveView_h
#define FireworksWeb_Core_FWEveView_h

namespace ROOT {
    namespace Experimental {
        class REveViewer;
        class REveScene;
        class REveViewContext;
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

  std::string viewType()const {return m_viewType;}

  virtual void importContext(ROOT::Experimental::REveViewContext*) {};
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
  //ROOT::Experimental::REveElement* m_ownedProducts;
  ROOT::Experimental::REveScene* m_geoScene{nullptr};
  std::string m_viewType;

private:
  FWEveView(const FWEveView&) = delete;                   // stop default
  const FWEveView& operator=(const FWEveView&) = delete;  // stop default

  // ---------- member data --------------------------------


};


class FW3DView : public FWEveView
{
public:
  FW3DView(std::string vtype);
  //~FW3DView() override;

  void  importContext(ROOT::Experimental::REveViewContext*) override;
};


class FWTableView : public FWEveView
{
public:
  FWTableView(std::string vtype);
  //~FW3DView() override;

  void  importContext(ROOT::Experimental::REveViewContext*) override;

};



#endif
