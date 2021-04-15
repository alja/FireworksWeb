#ifndef FireworksWeb_Core_FWMagField_h
#define FireworksWeb_Core_FWMagField_h
// -*- C++ -*-
//
// Simplified model of the CMS detector magnetic field
//

#include "ROOT/REveTrackPropagator.hxx"

class TH1F;

namespace edm
{
   class EventBase;
}

class FWMagField: public ROOT::Experimental::REveMagField
{
   double GetFieldMag() const;

public:
   enum ESource { kNone, kEvent, kUser };

   FWMagField();
   ~FWMagField() override;

   // get field values
   ROOT::Experimental::REveVectorD GetField(double x, double y, double z) const override;
   double    GetMaxFieldMag() const override;

   // auto/user behaviour
   void   setUserField(double b) { m_userField = b; }
   double  getUserField() const { return  m_userField; }
   void   setSource(ESource x) { m_source = x; }
   ESource  getSource() const { return m_source; }

   // field model
   void   setReverseState( bool state ){ m_reverse = state; }
   bool   isReverse() const { return m_reverse;}
   void   setSimpleModel( bool simpleModel ){ m_simpleModel = simpleModel; }
   bool   isSimpleModel() const { return m_simpleModel;}

   // field estimate
   void   guessFieldIsOn( bool guess ) const;
   void   guessField( double estimate ) const;
   void   resetFieldEstimate() const;

   void   checkFieldInfo(const edm::EventBase*);
   void   setFFFieldMag(double);

private:
   FWMagField(const FWMagField&) = delete; // stop default
   const FWMagField& operator=(const FWMagField&) = delete; // stop default

   ESource   m_source;
   double     m_userField;
   double     m_eventField;

   bool   m_reverse;
   bool   m_simpleModel;

   // runtime estimate , have to be mutable becuse of GetField() is const
   mutable TH1F  *m_guessValHist;
   mutable int    m_numberOfFieldIsOnEstimates;
   mutable int    m_numberOfFieldEstimates;
   mutable bool   m_updateFieldEstimate;
   mutable double  m_guessedField;
};

#endif
