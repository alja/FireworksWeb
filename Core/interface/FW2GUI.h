#ifndef FireworksWeb_Core_FW2GUI_h
#define FireworksWeb_Core_FW2GUI_h

#include "ROOT/REveElement.hxx"
#include "ROOT/REveManager.hxx"

#include "FireworksWeb/Core/interface/FW2Main.h"

class FW2Main;

namespace fwlite {
class Event;
}

class FW2GUI : public ROOT::Experimental::REveElement
{
public:
   typedef std::vector<std::string> CtrlStates_t;
private:
   FW2Main* m_main;

   float m_playdelay{500}; // 500ms = 0.5 sec cached
   
   CtrlStates_t m_ctrlStates;

public:
   FW2GUI();
   FW2GUI(FW2Main*);
   virtual ~FW2GUI();

   void terminate();
   
   void NextEvent();
   void PreviousEvent();
   void FirstEvent();
   void LastEvent();
   void goToRunEvent(int, int, int);

   CtrlStates_t& refCtrlStates() { return m_ctrlStates;}

   void requestConfiguration();
   void saveConfigurationAs(const char*);

   void setAutoplay(bool);
   void setPlayDelayInMiliseconds(int);


   float getPlayDelayInMiliseconds() const { return m_playdelay;}

   void RequestAddCollectionTable();
   void AddCollection(bool isEDM,const char* purpose, const char* moduleLabel, const char* productInstanceLabel,
                     const char* processName, const char* type);

   int WriteCoreJson(nlohmann::json &j, int rnr_offset) override;

};

#endif
