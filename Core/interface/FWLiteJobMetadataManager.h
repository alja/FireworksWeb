#ifndef Fireworks2_Core_FWLiteJobMetadataManager
#define Fireworks2_Core_FWLiteJobMetadataManager

#include "Fireworks2/Core/interface/FWJobMetadataManager.h"

namespace fwlite
{
   class Event;
}

class FWJobMetadataUpdateRequest;

class FWLiteJobMetadataManager : public FWJobMetadataManager
{
public:
   FWLiteJobMetadataManager(void);
   bool doUpdate(FWJobMetadataUpdateRequest *request) override;

   bool  hasModuleLabel(std::string& moduleLabel) override;

private:
   const fwlite::Event *m_event;
};

#endif
