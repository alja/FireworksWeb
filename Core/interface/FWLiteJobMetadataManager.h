#ifndef FireworksWeb_Core_FWLiteJobMetadataManager
#define FireworksWeb_Core_FWLiteJobMetadataManager

#include "FireworksWeb/Core/interface/FWJobMetadataManager.h"
#include "DataFormats/Provenance/interface/ProductDescriptionFwd.h"

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

   void matchAssociations(const edm::ProductDescription&);
};

#endif
