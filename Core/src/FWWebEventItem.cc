// -*- C++ -*-
//
// Package:     Core
// Class  :     FWWebEventItem
//
// Implementation:
//     <Notes on implementation>
//
// Original Author:
//         Created:  Thu Jan  3 14:59:23 EST 2008
//

// system include files
#include <iostream>
#include <algorithm>
#include <exception>
#include <TClass.h>

#include "ROOT/REveManager.hxx"
#include "ROOT/REveScene.hxx"


// user include files
#include "DataFormats/FWLite/interface/Event.h"
#include "FireworksWeb/Core/interface/FWWebEventItem.h"
#include "FireworksWeb/Core/interface/FWItemAccessorBase.h"
#include "FireworksWeb/Core/interface/fwLog.h"

#include "FireworksWeb/Core/interface/FWConfiguration.h"
#include "FireworksWeb/Core/interface/FWParameterizable.h"
#include "FireworksWeb/Core/interface/FWProxyBuilderConfiguration.h"

#include "FireworksWeb/Core/interface/FWGenericHandle.h"
//
//
// constructors and destructor
//
FWWebEventItem::FWWebEventItem(std::shared_ptr<FWItemAccessorBase> iAccessor,
                         const FWPhysicsObjectDesc& iDesc, const FWConfiguration* pbc, FW2EveManager* eveMng) :
   m_accessor(iAccessor),
   m_name(iDesc.name()),
   m_type(iDesc.type()),
   m_purpose(iDesc.purpose()),
   m_displayProperties(iDesc.displayProperties()),
   m_moduleLabel(iDesc.moduleLabel()),
   m_productInstanceLabel(iDesc.productInstanceLabel()),
   m_processName(iDesc.processName()),
   m_event(nullptr),
   m_printedErrorThisEvent(false)
{
  // m_collection = new ROOT::Experimental::REveDataCollection();
   //m_collection->SetName(iDesc.name());
   SetName(iDesc.name());
   SetLayer(iDesc.layer()*4);
   std::cout << "collection " << GetName() << ", layer " << GetLayer() << "\n"; 

   std::string title = m_moduleLabel + std::string(" ") + iDesc.type()->GetName();
  SetTitle(title.c_str());
  SetItemClass((TClass*)iAccessor->modelType());
  SetMainColor(iDesc.displayProperties().color());
   if (!iDesc.filterExpression().empty())
     SetFilterExpr(iDesc.filterExpression().c_str());

  SetRnrSelf(iDesc.displayProperties().isVisible());
   auto sl = ROOT::Experimental::gEve->GetScenes();
   auto cs = sl->FindChild("Collections");
   
   cs->AddElement(this);

   m_proxyBuilderConfig = new FWProxyBuilderConfiguration(pbc, this, eveMng);
}
// FWWebEventItem::FWWebEventItem(const FWWebEventItem& rhs)
// {
//    // do actual copying here;
// }

FWWebEventItem::~FWWebEventItem()
{
}

//
// assignment operators
//
// const FWWebEventItem& FWWebEventItem::operator=(const FWWebEventItem& rhs)
// {
//   //An exception safe implementation is
//   FWWebEventItem temp(rhs);
//   swap(rhs);
//
//   return *this;
// }

//
// member functions
//
void
FWWebEventItem::setEvent(const edm::EventBase* iEvent)
{
   m_event = iEvent;
   m_printedErrorThisEvent = false;

   m_accessor->reset();

   data();
   StampObjProps(); // AMT, it seems to me there is a bug in summary view. Alert icon is missing
}

//
// const member functions
//
const void*
FWWebEventItem::data()
{
   //lookup data if we don't already have it
   if (m_accessor->data())
      return m_accessor->data();

   m_errorMessage.clear();
   if (!m_event)
      return m_accessor->data();

   // remove all elements in REveDataCollection
   ClearItems();

   // Retrieve the data from the event.
   edm::InputTag tag(m_moduleLabel, m_productInstanceLabel, m_processName);
   edm::TypeWithDict type(*(m_type->GetTypeInfo()));
   edm::FWGenericHandle handle(type);
   try
   {
      m_event->getByLabel(tag, handle);
      setData(*handle);
   }
   catch (std::exception& iException)
   {
      if (!m_printedErrorThisEvent)
      {
         std::ostringstream s;
         s << "Failed to get " << m_name << " because \n" <<iException.what();
         m_errorMessage=s.str();
         fwLog(fwlog::kError) << s.str() << "\n";
         m_printedErrorThisEvent = true;
      }
      return nullptr;
   }

   return m_accessor->data();
}

void
FWWebEventItem::setData(const edm::ObjectWithDict& iData)
{
   m_accessor->setData(iData);
   
   // std::cout << "FWWebEventItem::setData " << GetName() << " size " << m_accessor->size() << std::endl;
   for (size_t i = 0; i < m_accessor->size(); ++i)
   {
      std::string cname = GetName();
      auto len = cname.size();
      char end = cname[len-1];
      if (end == 's') {
         cname = cname.substr(0, len-1);
      }
      TString pname(Form("%s %2d",  cname.c_str(), (int)i));
      AddItem( (void*)m_accessor->modelData(i), pname.Data(), pname.Data() );
   }
   ApplyFilter();
}

const TClass*
FWWebEventItem::modelType() const
{
   return m_accessor->modelType();
}
/*
void
FWWebEventItem::getPrimaryData() const
{
   //if(0!=m_data) return;
   if(nullptr!=m_accessor->data()) return;
   this->data(*(m_type->GetTypeInfo()));
}
*/
/*
bool
FWWebEventItem::isCollection() const
{
   return m_accessor->isCollection();
}
*/

/*
const void*
FWWebEventItem::modelData(int iIndex) const
{
   // this could also be retrived from REveDataCollection
   getPrimaryData();
   return m_accessor->modelData(iIndex);
}
*/



const TClass*
FWWebEventItem::type() const
{
   return m_type;
}

const std::string&
FWWebEventItem::purpose() const
{
   return m_purpose;
}

const std::string&
FWWebEventItem::moduleLabel() const
{
   return m_moduleLabel;
}
const std::string&
FWWebEventItem::productInstanceLabel() const
{
   return m_productInstanceLabel;
}

const std::string&
FWWebEventItem::processName() const
{
   return m_processName;
}

const char*
FWWebEventItem::name() const
{
   return GetCName();
}


const char*
FWWebEventItem::filterExpression()
{
  return  GetFilterExpr();
}


const FWDisplayProperties&
FWWebEventItem::defaultDisplayProperties() const
{
   return m_displayProperties;
}

int FWWebEventItem::WriteCoreJson(nlohmann::json &j, int rnr_offset)
{
   int res = REveDataCollection::WriteCoreJson(j, -1);

   m_proxyBuilderConfig->writeJson(j);


   nlohmann::json f{
          {"t", m_type->GetName()},
          {"m", m_moduleLabel},
          {"prod", m_productInstanceLabel},
          {"proc", m_processName}};
          
   j["edmInfo"] = f;
   if (!m_errorMessage.empty()) j["err"] = m_errorMessage;
  //  std::cout << " FWWebEventItem::WriteCoreJson " << j.dump(4);
   return res;
}


void FWWebEventItem::UpdatePBParameter(char* name, char* val)
{
   printf("Udate PB paramter %s %s \n", name, val);
   m_proxyBuilderConfig->setFromMIR(name, val);
   StampObjProps();
/*
      Ids_t ids;
      for (int i = 0; i < GetNItems(); ++i) 
      ids.push_back(i);
*/
}
