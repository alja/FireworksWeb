#ifndef Fireworks2_Calo_scaledMarker_h
#define Fireworks2_Calo_scaledMarker_h

namespace ROOT {
namespace Experimental {
class TEveScalableStraightLineSet;
//class FWViewContext;
}
}

namespace fireworks
{
struct scaleMarker {
   scaleMarker(ROOT::Experimental::REveScalableStraightLineSet* ls, float et, float e):
      m_ls(ls),
      m_et(et),
      m_energy(e)
   {
   };

   virtual ~scaleMarker() {}

   ROOT::Experimental::EveScalableStraightLineSet* m_ls;
   float m_et;
   float m_energy;
   // const FWViewContext* m_vc;
};
}

#endif
