#ifndef FireworksWeb_Core_FWColorManagerExtra_h
#define FireworksWeb_Core_FWColorManagerExtra_h
#include "FireworksWeb/Core/interface/FWColorManager.h"

namespace fireworks {
  void GetColorValuesForPaletteExtra(float (*iColors)[3], unsigned int iSize, FWColorManager::EPalette id, bool isBlack);
}
#endif
