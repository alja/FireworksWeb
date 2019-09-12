#ifndef Fireworks2_Core_FW2Main_h
#define Fireworks2_Core_FW2Main_h

class TTree;
class TFile;

namespace ROOT {
namespace Experimental  {
class REveScene;
}
}

namespace fwlite {
class Event;
}

class FW2EventManager;
class FWEventItem;
class FWItemAccessorFactory;
class FWLiteJobMetadataManager;
class FW2EveManager;
class FW2GUI;
class FWPhysicsObjectDesc;


class FW2Main
{
public:
   TFile              *m_file;
   TTree              *m_event_tree;
   fwlite::Event      *m_event;
   ROOT::Experimental::REveScene     *m_collections;
   FW2EveManager      *m_eveMng;
   FW2GUI             *m_gui;

   //------------------------------------------------------

   FW2Main(int argc, char *argv[]);
   ~FW2Main();

   void nextEvent();
   void previousEvent();
   
   void goto_event(Long64_t);
   void addTestItems();

   const fwlite::Event* getCurrentEvent() const { return m_event; }
   FWLiteJobMetadataManager* getMetadataManager() { return m_metadataManager; }
   // FW2EventManager* getFW2EveManger() { return m_eveMng; }
   void addFW2Item(FWPhysicsObjectDesc&);
   
private:
   FWItemAccessorFactory* m_accessorFactory;
   std::vector <FWEventItem*> m_items;

   FWLiteJobMetadataManager*   m_metadataManager;
   Long64_t m_eventId;
   
   std::vector<std::string> m_inputFiles;
};



#endif
