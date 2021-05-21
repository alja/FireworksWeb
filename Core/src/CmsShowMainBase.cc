#include "FireworksWeb/Core/interface/CmsShowMainBase.h"
#include "TROOT.h"

CmsShowMainBase::CmsShowMainBase()
{
    gROOT->SetBatch(true);
}

void CmsShowMainBase::setPlayLoop(bool x)
{
      m_loop = x;
}

