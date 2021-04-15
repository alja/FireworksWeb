#ifndef FireworksWeb_Core_FWLiteJobMetadataUpdateRequest
#define FireworksWeb_Core_FWLiteJobMetadataUpdateRequest

#include "FireworksWeb/Core/interface/FWJobMetadataUpdateRequest.h"

namespace fwlite
{
   class Event;
}

class TFile;


class FWLiteJobMetadataUpdateRequest : public FWJobMetadataUpdateRequest
{
public:
   FWLiteJobMetadataUpdateRequest(const fwlite::Event *event, 
                                  const TFile *file)
      : event_(event), file_(file)
   {
      
   }
   
   const fwlite::Event *event_;
   const TFile *file_;
};

#endif
