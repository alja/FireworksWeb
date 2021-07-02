// -*- C++ -*-
//
// Package:     Core
// Class  :     fwLog
//
// Implementation:
//     [Notes on implementation]
//
// Original Author:  Chris Jones
//         Created:  Tue Dec  8 23:10:10 CST 2009
//

// system include files

// user include files
#include "FireworksWeb/Core/interface/fwLog.h"


//
// constants, enums and typedefs
//

//
// static data member definitions
//

namespace fwlog
{

LogLevel s_presentLevel = kInfo;

LogLevel presentLogLevel() {
   return s_presentLevel;
}

ROOT::Experimental::RLogChannel& getREveLog()
{
   auto rl = getRootLevel(s_presentLevel);
   if (rl != ROOT::Experimental::REveLog().GetVerbosity())
   ROOT::Experimental::REveLog().SetVerbosity(rl);

   return ROOT::Experimental::REveLog();
}

ROOT::Experimental::ELogLevel getRootLevel(LogLevel fwl)
{
   ROOT::Experimental::ELogLevel rl;
   switch (fwl)
   {
   case kDebug:
      rl = ROOT::Experimental::ELogLevel::kDebug;
      break;
   case kInfo:
      rl = ROOT::Experimental::ELogLevel::kInfo;
      break;
   case kWarning:
      rl = ROOT::Experimental::ELogLevel::kWarning;
      break;
   default:
      rl = ROOT::Experimental::ELogLevel::kError;
   }

   return rl;
}

void setPresentLogLevel(LogLevel iLevel) {
   s_presentLevel=iLevel;
}

}
