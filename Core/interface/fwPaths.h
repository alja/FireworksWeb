#ifndef FireworksWeb_Core_DataPath_h
#define FireworksWeb_Core_DataPath_h

#include <string>

class TString;

namespace fireworks {
  void setPath(TString& v);
  void getDecomposedVersion(const TString& s, int* out);
  int* supportedDataFormatsVersion();
  bool acceptDataFormatsVersion(TString& n);
  const std::string& clientVersion();
}  // namespace fireworks

#endif
