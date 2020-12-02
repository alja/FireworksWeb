#include <iostream>

#include "TH1F.h"
#include "Fireworks2/Core/interface/FWMagField.h"
#include "Fireworks2/Core/interface/fwLog.h"
#include "DataFormats/FWLite/interface/Handle.h"
#include "DataFormats/Common/interface/ConditionsInEdm.h"
#include "DataFormats/Scalers/interface/DcsStatus.h"
#include "DataFormats/FWLite/interface/Event.h"

using namespace ROOT::Experimental;

FWMagField::FWMagField() :
   REveMagField(),

   m_source(kNone),
   m_userField(-1),
   m_eventField(-1),

   m_reverse(true),
   m_simpleModel(false),

   m_guessValHist(nullptr),
   m_numberOfFieldIsOnEstimates(0),
   m_numberOfFieldEstimates(0),
   m_updateFieldEstimate(true),
   m_guessedField(0)
{
   m_guessValHist = new TH1F("FieldEstimations", "Field estimations from tracks and muons",
                             200, -4.5, 4.5);
   m_guessValHist->SetDirectory(nullptr);
}

FWMagField::~FWMagField()
{
   delete m_guessValHist;
}

//______________________________________________________________________________

REveVectorD
FWMagField::GetField(double x, double y, double z) const
{
   // Virtual method of REveMagField class.

   double R = sqrt(x*x+y*y);
   double field = m_reverse ? -GetFieldMag() : GetFieldMag();

   //barrel
   if ( TMath::Abs(z)<724 ){

      //inside solenoid
      if ( R < 300) return REveVector(0,0,field);
      // outside solinoid
      if ( m_simpleModel ||
           ( R>461.0 && R<490.5 ) ||
           ( R>534.5 && R<597.5 ) ||
           ( R>637.0 && R<700.0 ) )
         return REveVector(0,0,-field/3.8*1.2);

   } else {
      // endcaps
      if (m_simpleModel){
         if ( R < 50 ) return REveVector(0,0,field);
         if ( z > 0 )
            return REveVector(x/R*field/3.8*2.0, y/R*field/3.8*2.0, 0);
         else
            return REveVector(-x/R*field/3.8*2.0, -y/R*field/3.8*2.0, 0);
      }
      // proper model
      if ( ( ( TMath::Abs(z)>724 ) && ( TMath::Abs(z)<786 ) ) ||
           ( ( TMath::Abs(z)>850 ) && ( TMath::Abs(z)<910 ) ) ||
           ( ( TMath::Abs(z)>975 ) && ( TMath::Abs(z)<1003 ) ) )
      {
         if ( z > 0 )
            return REveVector(x/R*field/3.8*2.0, y/R*field/3.8*2.0, 0);
         else
            return REveVector(-x/R*field/3.8*2.0, -y/R*field/3.8*2.0, 0);
      }
   }
   return REveVector(0,0,0);
}

//______________________________________________________________________________

double
FWMagField::GetFieldMag() const
{
   float res;
   switch ( m_source )
   {
      case kEvent:
      {
         res = m_eventField;
         break;
      }
      case kUser:
      {
         res = m_userField;
         break;
      }
      default:
      {
         if ( m_updateFieldEstimate )
         {
            if ( m_guessValHist->GetEntries() > 2  && m_guessValHist->GetRMS()  < 0.5 )
            {
               m_guessedField = m_guessValHist->GetMean();


               // std::cout << "FWMagField::GetFieldMag(), get average "
               //  << m_guessValHist->GetMean() << " guessed value: RMS= "<< m_guessValHist->GetRMS()
               //  <<" samples "<< m_guessValHist->GetEntries() << std::endl;

            }
            else if ( m_numberOfFieldIsOnEstimates > m_numberOfFieldEstimates/2 || m_numberOfFieldEstimates == 0 )
            {
               m_guessedField = 3.8;
               // fwLog(fwlog::kDebug) << "FWMagField::GetFieldMag() get default field, number estimates "
               //  << m_numberOfFieldEstimates << " number fields is on  m_numberOfFieldIsOnEstimates" <<std::endl;
            }
            else
            {
               m_guessedField = 0;
               //  fwLog(fwlog::kDebug) << "Update field estimate, guess field is OFF." <<std::endl;
            }
            m_updateFieldEstimate  = false;
         }

         res = m_guessedField;
      }
   }

   return res;
}

double
FWMagField::GetMaxFieldMag() const
{
   // Runge-Kutta stepper does not like this to be zero.
   // Will be fixed in root.
   // The return value should definitley not be negative -- so Abs
   // should stay!

   return TMath::Max(TMath::Abs(GetFieldMag()), 0.01);
}

//______________________________________________________________________________

void FWMagField::guessFieldIsOn(bool isOn) const
{
   if ( isOn ) ++m_numberOfFieldIsOnEstimates;
   ++m_numberOfFieldEstimates;
   m_updateFieldEstimate  = true;
}

void FWMagField::guessField(double val) const
{
   // fwLog(filedDebug) <<  "FWMagField::guessField "<< val << std::endl;
   m_guessValHist->Fill(val);
   m_updateFieldEstimate = true;
}

void FWMagField::resetFieldEstimate() const
{
   m_guessValHist->Reset();
   m_guessValHist->SetAxisRange(-4, 4);
   m_numberOfFieldIsOnEstimates = 0;
   m_numberOfFieldEstimates = 0;
   m_updateFieldEstimate = true;
}

//______________________________________________________________________________
void FWMagField::checkFieldInfo(const edm::EventBase* event)
{
   const static float  currentToField = 3.8/18160;
   bool available = false;
   try
   {
      edm::InputTag conditionsTag("conditionsInEdm");
      edm::Handle<edm::ConditionsInRunBlock> runCond;
      // FIXME: ugly hack to avoid exposing an fwlite::Event. Need to ask
      //        Chris / Ianna how to get mag field from an EventBase.
      const fwlite::Event *fwEvent = dynamic_cast<const fwlite::Event*>(event);
      if (!fwEvent)
         return;

      m_source = kNone;
      fwEvent->getRun().getByLabel(conditionsTag, runCond);

      if( runCond.isValid())
      {
         available = true;
         m_eventField = currentToField * runCond->BAvgCurrent;
         m_source = kEvent;
         fwLog( fwlog::kDebug ) << "Magnetic field info found in ConditionsInEdm branch : "<< m_eventField << std::endl;
      }
      else
      {
         edm::InputTag dcsTag("scalersRawToDigi");
         edm::Handle< std::vector<DcsStatus> > dcsStatus;
         event->getByLabel(dcsTag, dcsStatus);

         if (dcsStatus.isValid() && !dcsStatus->empty())
         {
            float sum = 0;
            for (std::vector<DcsStatus>::const_iterator i = dcsStatus->begin(); i <  dcsStatus->end(); ++i)
               sum += (*i).magnetCurrent();

            available = true;
            m_eventField = currentToField * sum/dcsStatus->size();
            m_source = kEvent;
            fwLog( fwlog::kDebug) << "Magnetic field info found in DcsStatus branch: " << m_eventField << std::endl;
         }

      }
   }
   catch (cms::Exception&)
   {
      fwLog( fwlog::kDebug ) << "Cought exception in FWMagField::checkFieldInfo\n";
   }

   if (!available)
   {
      fwLog( fwlog::kDebug ) << "No magnetic field info available in Event\n";
   }
}


//______________________________________________________________________________
void FWMagField::setFFFieldMag(double mag)
{
   // AMT this is a workaround for seting FF in FFLooper
   // Correct imeplementation is having a base class of  FWMagField amd do implementation for FF and FWLite version

   m_source = kEvent;
   m_eventField = mag;
}
