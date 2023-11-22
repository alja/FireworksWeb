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
private:
   FW2Main* m_main;
   void autoplay_scheduler();
   std::chrono::duration<double> m_deltaTime{1};

   std::thread *m_timerThread{nullptr};
   std::mutex m_mutex;
   std::condition_variable m_CV;

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

   void requestConfiguration();
   void saveConfigurationAs(const char*);

   void autoplay(bool);
   void playdelay(float);


   bool getAutoplay() const { return m_autoplay; }

   void RequestAddCollectionTable();
   void AddCollection(bool isEDM,const char* purpose, const char* moduleLabel, const char* productInstanceLabel,
                     const char* processName, const char* type);

   int WriteCoreJson(nlohmann::json &j, int rnr_offset) override;

   bool m_autoplay{false};
};

#endif
