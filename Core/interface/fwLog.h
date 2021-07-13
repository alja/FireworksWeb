#ifndef FireworksWeb_Core_fw2Log_h
#define FireworksWeb_Core_fw2Log_h
#include "ROOT/RLogger.hxx"
#include "ROOT/REveTypes.hxx"
// -*- C++ -*-
//
// Package:     Core
// Class  :     fwLog
//
/**\class fwLog fwLog.h FireworksWeb/Core/interface/fwLog.h

 Description: Simple logging utility

 Usage:
    To send a message to the logger
       fwLog(kDebug) << "This is my message"<<std::endl;

    To change the message levels which will be recorded
       fwlog::setPresentLevel(kDebug);

    To change where the messages go, just pass the address of an instance of std::ostream
       fwlog::setLogger(&std::cerr);

*/
//
// Original Author:  Chris Jones
//         Created:  Tue Dec  8 23:10:04 CST 2009
//

namespace fwlog {
   enum LogLevel {
      kDebug,
      kInfo,
      kWarning,
      kError
   };

   const char* levelName(LogLevel);
   std::ostream& logger();
   void setLogger(std::ostream*);

   LogLevel presentLogLevel();
   void setPresentLogLevel(LogLevel);


   ROOT::Experimental::RLogChannel &getREveLog();
   ROOT::Experimental::ELogLevel getRootLevel(LogLevel);
}

#define fwLogOrig(_level_) \
   (fwlog::presentLogLevel() > _level_) ? fwlog::logger() : fwlog::logger()<< fwlog::levelName(_level_)<<": "

#define fwLog(_level_) \
   R__LOG_TO_CHANNEL(fwlog::getRootLevel(_level_), fwlog::getREveLog()) << ":"

#endif // fwLog.h source 
