#ifndef Fireworks2_Core_FW2GUI_h
#define Fireworks2_Core_FW2GUI_h

#include "ROOT/REveElement.hxx"
#include "ROOT/REveManager.hxx"

#include "Fireworks2/Core/interface/FW2Main.h"

class FW2Main;

namespace fwlite {
class Event;
}

class FW2GUI : public ROOT::Experimental::REveElement
{
private:
   FW2Main* m_main;
public:
   FW2GUI();
   FW2GUI(FW2Main*);
   virtual ~FW2GUI();

   void NextEvent();
   void PreviousEvent();
   void RequestAddCollectionTable();
   void AddCollection(const std::string& purpose, const std::string& label, const std::string& process, const std::string& type);

   // void sendEventInfo(const fwlite::Event*);
   int WriteCoreJson(nlohmann::json &j, int rnr_offset) override;

   int m_ecnt; // cached, this will move to FWFileEntry
};

#endif
