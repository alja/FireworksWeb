#include "FireworksWeb/Core/interface/FWLiteJobMetadataManager.h"
#include "FireworksWeb/Core/interface/FWLiteJobMetadataUpdateRequest.h"
#include "FireworksWeb/Core/interface/fwLog.h"
#include "FireworksWeb/Core/interface/FWItemAccessorFactory.h"
#include "DataFormats/Provenance/interface/BranchDescription.h"
#include "DataFormats/FWLite/interface/Event.h"

#include "TFile.h"
#include "TTree.h"
#include "TBranchElement.h"
#include <set>

FWLiteJobMetadataManager::FWLiteJobMetadataManager(void)
   : FWJobMetadataManager(),
     m_event(nullptr)
{}

bool
FWLiteJobMetadataManager::hasModuleLabel(std::string& moduleLabel)
{
   if (m_event) {
      for ( auto bit = m_event->getBranchDescriptions().begin(); bit !=  m_event->getBranchDescriptions().end(); ++bit)
      {
         if (bit->moduleLabel() == moduleLabel) {
            return true;
         }
      }
   }
   return false;
}



namespace {
TBranchElement *findFinalBranch(TBranchElement *be)
{
   if (be->GetListOfBranches()->IsEmpty())
      return be;

   TIter iObj(be->GetListOfBranches());
   while (TBranchElement* ch = (TBranchElement*)iObj()) {
      auto sub = findFinalBranch(ch);
      if (sub)
         return sub;
   }
   return nullptr;
}

bool isEmptySplit(TBranchElement *be)
{
   auto sub = findFinalBranch(be);
   if (sub) {
      if (sub->GetType() != 31) {
         // printf("Internal Error found a 'final' branch that is not of type 31\n");
         return false;
      }
      auto expected_size = be->GetWriteBasket() * ( 63 + (long)strlen(be->GetName()) + (long)strlen(be->GetTree()->GetName()) + 8 /* offset array index + extra element */ ) + 4 * be->GetEntries() ;
      bool isEmptySplit = (be->GetTotBytes() == expected_size);
      /*
      printf("Final branch (%s) TotBytes = %lld vs %lld, sEmptySplit = %d\n",  be->GetName(), be->GetTotBytes(),
             expected_size, isEmptySplit);*/
      return isEmptySplit;
   }
   return false;
}

bool isEmptySubBranch(TBranch* b)
{
   TBranchElement *bre = dynamic_cast<TBranchElement*>(b);
   if (!bre)
      return false;

   // printf("isEmptySubBranch test:  %s\n", b->GetName());
   if (bre->GetType() == 3) {
      // PC split collection, let's look for an inner branch.
      return isEmptySplit(bre);
   }

   if (bre->GetStreamerType() == 300) {
      // PC unsplit collection.
      auto expected_size = b->GetWriteBasket() * ( 63 + (long)strlen(b->GetName()) + (long)strlen(b->GetTree()->GetName()) + 8 /* offset array index + extra element */ ) + 16 * b->GetEntries() ;
      bool isEmptyNonSplit = (b->GetTotBytes() == expected_size);
      /*
      printf("Final branch (%s) TotBytes = %lld vs %lld, %d %ld %lld, isEmptyNonSplit = %d\n",  b->GetName(), b->GetTotBytes(),
             expected_size,
             b->GetWriteBasket(), (long)strlen(b->GetName()), b->GetEntries(),
             isEmptyNonSplit);
             */
      return isEmptyNonSplit;
   }

   // Not a collection
   return false;
}

bool isEmpty(TBranch *b)
{
   std::string obj_branchname = std::string(b->GetName())+"obj";
   auto objbr = dynamic_cast<TBranch*>(b->GetListOfBranches()->FindObject(obj_branchname.c_str()));
   if (!objbr)
      obj_branchname += ".";
   if (objbr)
      return isEmptySubBranch(objbr);
   else
      return false;
}
}


/** This method inspects the currently opened TFile and for each branch 
    containing products for which we can either build a TCollectionProxy or 
    for which we have a specialized accessor, it registers it as a viewable 
    item.
 */
bool
FWLiteJobMetadataManager::doUpdate(FWJobMetadataUpdateRequest *request)
{
   FWLiteJobMetadataUpdateRequest *liteRequest 
      = dynamic_cast<FWLiteJobMetadataUpdateRequest *>(request);
   // There is no way we are going to get a non-FWLite updated request for
   // this class.
   assert(liteRequest);
   if (m_event == liteRequest->event_) 
      return false;

   m_event = liteRequest->event_;
   const TFile *file = liteRequest->file_;

   assert(file);
   
   usableData().clear();
   
   if (!m_event)
      return true;
   
   const std::vector<std::string>& history = m_event->getProcessHistory();
   
   // Turns out, in the online system we do sometimes gets files without any  
   // history, this really should be investigated
   if (history.empty())
      std::cout << "WARNING: the file '"
         << file->GetName() << "' contains no processing history"
            " and therefore should have no accessible data.\n";
   
   std::copy(history.rbegin(),history.rend(),
             std::back_inserter(processNamesInJob()));
   
   static const std::string s_blank;
   const std::vector<edm::BranchDescription>& descriptions =
      m_event->getBranchDescriptions();

   Data d;
   
   //I'm not going to modify TFile but I need to see what it is holding
   TTree* eventsTree = dynamic_cast<TTree*>(const_cast<TFile*>(file)->Get("Events"));
   assert(eventsTree);
   
   std::map<std::string, bool> branchNamesInFile;
   TIter nextBranch(eventsTree->GetListOfBranches());
   while(TBranch* branch = static_cast<TBranch*>(nextBranch()))
   {
      bool bad = isEmpty(branch);
      branchNamesInFile[branch->GetName()]  = bad;
   }

   typedef std::set<std::string> Purposes;
   Purposes purposes;
   std::string classType;

   
   
   for(size_t bi = 0, be = descriptions.size(); bi != be; ++bi) 
   {
      const edm::BranchDescription &desc = descriptions[bi];
      
      if (!desc.present() 
          || branchNamesInFile.end() == branchNamesInFile.find(desc.branchName()))
          {
         continue;
          }
      const std::vector<FWRepresentationInfo>& infos 
         = m_typeAndReps->representationsForType(desc.fullClassName());
      
      /*
      std::cout <<"try to find match "<<desc.fullClassName()<<std::endl;
      //For each view we need to find the non-sub-part builder whose proximity is smallest and 
      // then register only that purpose
      //NOTE: for now, we will ignore the view and only look for the closest proximity
      unsigned int minProx = ~(0U);
      for (size_t ii = 0, ei = infos.size(); ii != ei; ++ii) {
      if (!infos[ii].representsSubPart() && minProx > infos[ii].proximity()) {
      minProx = infos[ii].proximity();
      }
      }
      */
      
      //the infos list can contain multiple items with the same purpose so we will just find
      // the unique ones
      purposes.clear();
      for (size_t ii = 0, ei = infos.size(); ii != ei; ++ii) {
         if (infos[ii].requiresFF() == false) {
             purposes.insert(infos[ii].purpose());
         }
      }
      
      if (purposes.empty())
         purposes.insert("Table");
      
      for (Purposes::const_iterator itPurpose = purposes.begin(),
              itEnd = purposes.end();
           itPurpose != itEnd;
           ++itPurpose) 
      {
         // Determine whether or not the class can be iterated
         // either by using a TVirtualCollectionProxy (of the class 
         // itself or on one of its members), or by using a 
         // FWItemAccessor plugin.
         TClass* theClass = TClass::GetClass(desc.fullClassName().c_str());
         
         if (!theClass)
            continue;
      
         if (!theClass->GetTypeInfo())
            continue;
         
         const static bool debug = false;
         // This is pretty much the same thing that happens 
         if (!FWItemAccessorFactory::classAccessedAsCollection(theClass) )
         {
            if (debug) {
               fwLog(fwlog::kDebug) << theClass->GetName() 
                                    << " will not be displayed in table." << std::endl;
            }
            continue;
         }
         d.type_ = desc.fullClassName();
         d.purpose_ = *itPurpose;
         d.moduleLabel_ = desc.moduleLabel();
         d.productInstanceLabel_ = desc.productInstanceName();
         d.processName_ = desc.processName();
         d.empty_ = branchNamesInFile[desc.branchName()];
         usableData().push_back(d);
         if (debug)
         {
            fwLog(fwlog::kDebug) << "Add collection will display " << d.type_
                                 << " " << d.purpose_
                                 << " " << d.moduleLabel_ 
                                 << " " << d.productInstanceLabel_
                                 << " " << d.processName_ << std::endl;
         }
      }

      if (infos.empty())
         matchAssociations(desc);
   }
   return true;
}

void FWLiteJobMetadataManager::matchAssociations(const edm::BranchDescription &desc)
{
   bool debug = false;
   const std::string& bdcl = desc.className();
   // check if it begins with edm::Assoc
   if (bdcl.length() > 5 && bdcl.substr(5, 5) == "Assoc")
   {
      for (auto const &a : m_associationTypes)
      {
         try {
               if (debug)
               {
                  std::cout << "comapre \n"
                              << desc.unwrappedTypeID().typeInfo().name() << "\n"
                              << a << "\n---\n";
               }
               std::string brName = desc.unwrappedTypeID().typeInfo().name();
               size_t found = a.rfind(brName, 0);
               if (found != std::string::npos )
               {
                  Data d;
                  d.type_ = desc.className();
                  size_t bl = brName.length();
                  std::string ph = a.substr(bl + 1);
                  d.purpose_ = ph.substr(0, ph.rfind("#"));
                  d.moduleLabel_ = desc.moduleLabel();
                  d.productInstanceLabel_ = desc.productInstanceName();
                  d.processName_ = desc.processName();
                  d.isEDM = false;
                  usableData().push_back(d);
               }
            }
            catch (edm::Exception& e)
           {
          fwLog(fwlog::kInfo) << "FWLiteJobMetadataManager::matchAssociations " << e.what() << "\n";
           }
      }
   }
}
