#include "FireworksWeb/Core/interface/fwPaths.h"
#include "FireworksWeb/Core/interface/fwLog.h"
#include "TString.h"
#include "TSystem.h"
#include "TPRegexp.h"
#include <iostream>
#include <fstream>
#include <type_traits>
#include "DataFormats/TrackReco/interface/TrackFwd.h"

// Since CMSSW_20_X reco::Track is an alias of reco::io_v1::Track. In older releases this
// declaration introduces an unrelated class, so the comparison below detects the data formats
// generation at compile time.
namespace reco::io_v1 {
  class Track;
}

namespace fireworks {

  const TString datadir("/src/FireworksWeb/Core/");
  const TString p1 = gSystem->Getenv("CMSSW_BASE") + datadir;
  const TString p2 = gSystem->Getenv("CMSSW_RELEASE_BASE") + datadir;

  void setPath(TString& v) {
    if (gSystem->AccessPathName(p1 + v) == kFALSE) {
      v.Prepend(p1);
      return;
    }

    v.Prepend(p2);
    if (gSystem->AccessPathName(v))
      fwLog(fwlog::kError) << "Can't access path " << v << std::endl;
  }

  void getDecomposedVersion(const TString& s, int* out) {
    TPMERegexp re("CMSSW_(\\d+)_(\\d+)_");
    re.Match(s);
    if (re.NMatches() > 2) {
      out[0] = re[1].Atoi();
      out[1] = re[2].Atoi();
    }
  }

  int* supportedDataFormatsVersion() {
    static int mm[] = {0, 0, 0};

    if (!mm[0]) {
      TString v;
      if (gSystem->Getenv("CMSSW_VERSION")) {
        v = gSystem->Getenv("CMSSW_VERSION");

      } else {
        TString versionFileName = gSystem->Getenv("CMSSW_BASE");
        versionFileName += "/src/FireworksWeb/Core/data/version.txt";
        std::ifstream fs(versionFileName);
        TString infoText;
        infoText.ReadLine(fs);
        infoText.ReadLine(fs);
        fs.close();
        v = &infoText[13];
      }

      getDecomposedVersion(v, &mm[0]);
    }

    return &mm[0];
  }

  bool ioV1DataFormats() { return std::is_same_v<reco::Track, reco::io_v1::Track>; }

  bool acceptDataFormatsVersion(TString& processConfigurationVersion) {
    int data[] = {0, 0, 0};
    getDecomposedVersion(processConfigurationVersion, data);

    // int* running = supportedDataFormatsVersion();
    // data formats with io_v1 namespace are incompatible with the ones before CMSSW_20_X
    bool res = ioV1DataFormats() ? (data[0] >= 20) : (data[0] > 5 && data[0] < 20);
    return res;
  }

  const std::string& clientVersion()
  {
    const static std::string cv = "33.03";
    return cv;
  }

}  // namespace fireworks
