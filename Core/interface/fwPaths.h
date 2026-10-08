#ifndef FireworksWeb_Core_DataPath_h
#define FireworksWeb_Core_DataPath_h

#include <string>

class TString;

namespace fireworks {
  void setPath(TString& v);
  void getDecomposedVersion(const TString& s, int* out);
  int* supportedDataFormatsVersion();
  bool acceptDataFormatsVersion(TString& n);
  // true when built against CMSSW_20_X or later data formats, where reco classes live in reco::io_v1
  bool ioV1DataFormats();
  const std::string& clientVersion();
}  // namespace fireworks

#endif
