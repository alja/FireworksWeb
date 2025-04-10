#ifndef FireworksWeb_Core_FWEventAnnotation_h
#define FireworksWeb_Core_FWEventAnnotation_h


namespace fwlite {
  class Event;
}

namespace ROOT{
  namespace Experimental 
  {
    class REveElement;
    class REveText;
  }
}

class FWEventAnnotation {
public:
  FWEventAnnotation(ROOT::Experimental::REveElement*);
  ~FWEventAnnotation();


  //configuration management interface
  //virtual void addTo(FWConfiguration&) const;
  //virtual void setFrom(const FWConfiguration&);

  void setLevel(long x);
  void setEvent();
  void bgChanged(bool is_dark);
  FWEventAnnotation(const FWEventAnnotation&) = delete;                   // stop default
  const FWEventAnnotation& operator=(const FWEventAnnotation&) = delete;  // stop default

private:
  void updateOverlayText();
  ROOT::Experimental::REveText* assertEveText();

  int m_level;
  ROOT::Experimental::REveText* m_eveText{nullptr};
  ROOT::Experimental::REveElement* m_holder{nullptr};
};

#endif
