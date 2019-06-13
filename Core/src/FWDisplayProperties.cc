// -*- C++ -*-
//
// Package:     Core
// Class  :     FWDisplayProperties
//
// Implementation:
//     <Notes on implementation>
//
// Original Author:
//         Created:  Thu Jan  3 17:05:44 EST 2008
//

// system include files

// user include files
#include "Fireworks2/Core/interface/FWDisplayProperties.h"


// A static default property.
const FWDisplayProperties FWDisplayProperties::defaultProperties
(1, true, 0);

FWDisplayProperties::FWDisplayProperties(Color_t iColor,
                                         bool    isVisible,
                                         Char_t  transparency) 
   : m_color(iColor),
     m_isVisible(isVisible),
     m_transparency(transparency)
{}
