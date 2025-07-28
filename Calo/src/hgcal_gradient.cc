
#include "FWCore/Framework/interface/Event.h"
#include "DataFormats/FWLite/interface/Event.h"
#include "DataFormats/HGCRecHit/interface/HGCRecHitCollections.h"

#include "FireworksWeb/Calo/interface/hgcal_gradient.h"
#include "FireworksWeb/Core/interface/FWWebEventItem.h"
#include "FireworksWeb/Core/interface/FWProxyBuilderConfiguration.h"

#include "ROOT/REveDataCollection.hxx"

namespace fwhgcal
{
   const uint8_t gradient[3][FWHGCAL_GRADIENT_STEPS] = {{static_cast<uint8_t>(0.2082 * 255),
                                                           static_cast<uint8_t>(0.0592 * 255),
                                                           static_cast<uint8_t>(0.0780 * 255),
                                                           static_cast<uint8_t>(0.0232 * 255),
                                                           static_cast<uint8_t>(0.1802 * 255),
                                                           static_cast<uint8_t>(0.5301 * 255),
                                                           static_cast<uint8_t>(0.8186 * 255),
                                                           static_cast<uint8_t>(0.9956 * 255),
                                                           static_cast<uint8_t>(0.9764 * 255)},

                                                          {static_cast<uint8_t>(0.1664 * 255),
                                                           static_cast<uint8_t>(0.3599 * 255),
                                                           static_cast<uint8_t>(0.5041 * 255),
                                                           static_cast<uint8_t>(0.6419 * 255),
                                                           static_cast<uint8_t>(0.7178 * 255),
                                                           static_cast<uint8_t>(0.7492 * 255),
                                                           static_cast<uint8_t>(0.7328 * 255),
                                                           static_cast<uint8_t>(0.7862 * 255),
                                                           static_cast<uint8_t>(0.9832 * 255)},

                                                          {static_cast<uint8_t>(0.5293 * 255),
                                                           static_cast<uint8_t>(0.8684 * 255),
                                                           static_cast<uint8_t>(0.8385 * 255),
                                                           static_cast<uint8_t>(0.7914 * 255),
                                                           static_cast<uint8_t>(0.6425 * 255),
                                                           static_cast<uint8_t>(0.4662 * 255),
                                                           static_cast<uint8_t>(0.3499 * 255),
                                                           static_cast<uint8_t>(0.1968 * 255),
                                                           static_cast<uint8_t>(0.0539 * 255)
                                                           }
                                                           };


void  fillDetIdToHitMap(FWHgcalHitmap_t& hitmap) 
{
      hitmap.clear();

      const edm::EventBase* event = fireworks::Context::getInstance()->getCurrentEvent();

      {
        edm::Handle<HGCRecHitCollection> recHitHandleEE;
        event->getByLabel(edm::InputTag("HGCalRecHit", "HGCEERecHits"), recHitHandleEE);
        if (recHitHandleEE.isValid()) {
          const auto& rechitsEE = *recHitHandleEE;
          for (unsigned int i = 0; i < rechitsEE.size(); ++i) {
            (hitmap)[rechitsEE[i].detid().rawId()] = &rechitsEE[i];
          }
        }

        edm::Handle<HGCRecHitCollection> recHitHandleFH;
        event->getByLabel(edm::InputTag("HGCalRecHit", "HGCHEFRecHits"), recHitHandleFH);
        if (recHitHandleFH.isValid()) {
          const auto& rechitsFH = *recHitHandleFH;
          for (unsigned int i = 0; i < rechitsFH.size(); ++i) {
            (hitmap)[rechitsFH[i].detid().rawId()] = &rechitsFH[i];
          }
        }

        edm::Handle<HGCRecHitCollection> recHitHandleBH;
        event->getByLabel(edm::InputTag("HGCalRecHit", "HGCHEBRecHits"), recHitHandleBH);
        if (recHitHandleBH.isValid()) {
          const auto& rechitsBH = *recHitHandleBH;
          for (unsigned int i = 0; i < rechitsBH.size(); ++i) {
            (hitmap)[rechitsBH[i].detid().rawId()] = &rechitsBH[i];
          }
        }
    }


    // backward compatiblity for CMSS_13_X or older
    if (hitmap.empty())
    {
       edm::Handle< edm::SortedCollection <HGCRecHit,edm::StrictWeakOrdering<HGCRecHit> > >  recHitHandleEE;
        event->getByLabel(edm::InputTag("HGCalRecHit", "HGCEERecHits"), recHitHandleEE);
        if (recHitHandleEE.isValid()) {
          const auto& rechitsEE = *recHitHandleEE;
          for (unsigned int i = 0; i < rechitsEE.size(); ++i) {
            (hitmap)[rechitsEE[i].detid().rawId()] = &rechitsEE[i];
          }
        }

        edm::Handle< edm::SortedCollection <HGCRecHit,edm::StrictWeakOrdering<HGCRecHit> > > recHitHandleFH;
        event->getByLabel(edm::InputTag("HGCalRecHit", "HGCHEFRecHits"), recHitHandleFH);
        if (recHitHandleFH.isValid()) {
          const auto& rechitsFH = *recHitHandleFH;
          for (unsigned int i = 0; i < rechitsFH.size(); ++i) {
            (hitmap)[rechitsFH[i].detid().rawId()] = &rechitsFH[i];
          }
        }

        edm::Handle< edm::SortedCollection <HGCRecHit,edm::StrictWeakOrdering<HGCRecHit> > > recHitHandleBH;
        event->getByLabel(edm::InputTag("HGCalRecHit", "HGCHEBRecHits"), recHitHandleBH);
        if (recHitHandleBH.isValid()) {
          const auto& rechitsBH = *recHitHandleBH;
          for (unsigned int i = 0; i < rechitsBH.size(); ++i) {
            (hitmap)[rechitsBH[i].detid().rawId()] = &rechitsBH[i];
          }
        }
    }

}

void initProxyParameters(ROOT::Experimental::REveDataCollection* c)
{
      auto item = dynamic_cast<FWWebEventItem *>(c);
      // item->getConfig()->keepEntries(true);
      item->getConfig()->assertParam("Layer", 0L, 0L, 52L);
      item->getConfig()->assertParam("EnergyCutOff", 0.5, 0.2, 5.0);
      item->getConfig()->assertParam("Heatmap", true);
      item->getConfig()->assertParam("Z+", true);
      item->getConfig()->assertParam("Z-", true);
}

} // namespace

