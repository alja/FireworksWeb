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


   void autoplay_scheduler();
   std::chrono::duration<double> m_deltaTime{1};
   std::thread *m_timerThread{nullptr};
   std::mutex m_mutex;
   std::condition_variable m_CV;

   bool m_autoplay{false};
   float m_playdelay{500}; // 500ms = 0.5 sec
   
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
   void setPlayDelayInMiliseconds(float);


   bool getAutoplay() const { return m_autoplay; }
   float getPlayDelayInMiliseconds() const { return m_playdelay;}

   void RequestAddCollectionTable();
   void AddCollection(bool isEDM,const char* purpose, const char* moduleLabel, const char* productInstanceLabel,
                     const char* processName, const char* type);

   int WriteCoreJson(nlohmann::json &j, int rnr_offset) override;

};

#endif
