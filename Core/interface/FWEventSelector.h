// -*- C++ -*-
#ifndef FireworksWeb_Core_FWEventSelector_h
#define FireworksWeb_Core_FWEventSelector_h
//
// Package:     newVersion
// Class  :     FWEventSelector
//

// system include files
#include <string>

class FWEventSelector {
  public:
 // FWEventSelector(FWEventSelector* s) { *this = *s; }

  FWEventSelector() : m_enabled(false), m_selected(-1), m_updated(false) 
  {m_id = s_counter++; }


  FWEventSelector(int id) : m_enabled(false), m_selected(-1), m_updated(false) 
  {m_id = id;}

  std::string m_expression;
  std::string m_description;
  std::string m_triggerProcess;
  bool m_enabled;
  int m_selected;
  bool m_updated;

  int m_id{0};

  static int s_counter;
};
#endif
