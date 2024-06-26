#include "FireworksWeb/Core/interface/FWJobMetadataManager.h"
#include "FireworksWeb/Core/interface/FWJobMetadataUpdateRequest.h"
#include "FireworksWeb/Core/interface/FWAssociationManager.h"
#include <memory>
#include <cxxabi.h>


FWJobMetadataManager::FWJobMetadataManager(void)
   : m_typeAndReps(nullptr)
{}

FWJobMetadataManager::~FWJobMetadataManager()
{}

/** Invoked when a given update request needs to happen. Will
    emit the metadataChanged_ signal when done so that observers can 
    update accordingly.
    
    Derived classes should implement the doUpdate() protected method
    to actually modify the metadata according to the request.
    
    Notice that this method is a consumer of request object and takes
    ownership of the lifetime of the @a request objects.
  */
void
FWJobMetadataManager::update(FWJobMetadataUpdateRequest *request)
{
   std::unique_ptr<FWJobMetadataUpdateRequest> ptr(request);
   if (doUpdate(request)) {
      metadataChanged_();
   }
}

void
FWJobMetadataManager::initReps(const FWTypeToRepresentations& iTypeAndReps)
{
   delete m_typeAndReps;
   m_typeAndReps = new FWTypeToRepresentations(iTypeAndReps);
}

void
FWJobMetadataManager::initAssociationTypes(const FWAssociationManager* am)
{
   am->refAssociationTypes(m_associationTypes);
}
