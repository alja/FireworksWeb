#ifndef FireworksWeb_Vertex_FW2TrackProxyBuilder_h
#define FireworksWeb_Vertex_FW2TrackProxyBuilder_h

// -*- C++ -*-
//
// Package:     Vertexs
// Class  :     FWVertexProxyBuilder
//
// Implementation:
//     <Notes on implementation>
//
// Original Author:  Chris Jones
//         Created:  Tue Dec  2 14:17:03 EST 2008
//
// user include files// user include files

#include "FireworksWeb/Core/interface/FWWebEventItem.h"
#include "FireworksWeb/Core/interface/FWProxyBuilderFactory.h"
#include "FireworksWeb/Core/interface/fwLog.h"

#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"

#include "ROOT/REveViewContext.hxx"
#include "ROOT/REveVector.hxx"
#include "ROOT/REveTrans.hxx"
#include "ROOT/REveTrack.hxx"
#include "ROOT/REveTrackPropagator.hxx"
#include "ROOT/REveEllipsoid.hxx"
#include "ROOT/REvePointSet.hxx"
#include "ROOT/REveDataSimpleProxyBuilderTemplate.hxx"
#include "ROOT/REveDataSimpleProxyBuilderTemplate.hxx"
#include "FireworksWeb/Core/interface/FWProxyBuilderConfiguration.h"

#include "TMatrixDEigen.h"
#include "TMatrixDSym.h"
#include "TDecompSVD.h"
#include "TVectorD.h"
#include "TGeoSphere.h"

using namespace ROOT::Experimental;

class FWVertexProxyBuilder : public ROOT::Experimental::REveDataSimpleProxyBuilderTemplate<reco::Vertex>
{
public:
   REGISTER_FWPB_METHODS();

   using REveDataProxyBuilderBase::SetCollection;
   virtual void SetCollection(REveDataCollection* c)
   {
      REveDataProxyBuilderBase::SetCollection(c);
      auto item = dynamic_cast<FWWebEventItem *>(c);
      item->getConfig()->assertParam("Draw Tracks", false);
      item->getConfig()->assertParam("Draw Pseudo Track", false);
      item->getConfig()->assertParam("Draw Ellipse", false);
      item->getConfig()->assertParam("Scale Ellipse", 10l, 1l, 20l);
      REveDataProxyBuilderBase::Build();
   }

   using REveDataSimpleProxyBuilderTemplate<reco::Vertex>::BuildItem;
   virtual void BuildItem(const reco::Vertex &iData, int iIndex, ROOT::Experimental::REveElement *iItemHolder, const ROOT::Experimental::REveViewContext *vc) override
   {
      //std::cout << "vertex error \n" << iData.error() << std::endl;
      //printf("position %g, %g, %g \n", iData.x(), iData.y(), iData.z());
      reco::Vertex::Error e = iData.error();

      auto item = dynamic_cast<FWWebEventItem *>(Collection());
      auto context = fireworks::Context::getInstance();

      if (item->getConfig()->value<bool>("Draw Ellipse"))
      {
         TMatrixDSym xxx(3);
         for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
            {
               // printf("Read error [%d,%d] %g\n", i, j, e(i,j));
               xxx(i, j) = e(i, j);
            }
         //xxx.Print();

         TMatrixDEigen eig(xxx);
         TVectorD xxxEig(eig.GetEigenValues());
         //  xxxEig.Print();
         xxxEig = xxxEig.Sqrt();

         TMatrixD vecEig = eig.GetEigenVectors();
         // vecEig.Print();

         float scale = item->getConfig()->value<long>("Scale Ellipse");

         REveVector v[3];
         for (int i = 0; i < 3; ++i)
         {
            v[i].Set(vecEig(0, i), vecEig(1, i), vecEig(2, i));
            v[i] *= xxxEig(i) * scale;
         }
         REveEllipsoid *ell = new REveEllipsoid("VertexError");
         ell->RefMainTrans().SetPos(iData.x(), iData.y(), iData.z());
         ell->SetLineWidth(2);
         ell->SetBaseVectors(v[0], v[1], v[2]);
         ell->Outline();
         SetupAddElement(ell, iItemHolder);
      }

      // vertex position
      //
      auto ps = new REvePointSet("vertex pnt");
      ps->SetMainColor(kGreen + 10);
      ps->SetNextPoint(iData.x(), iData.y(), iData.z());
      ps->SetMarkerStyle(4);
      ps->SetMarkerSize(4);
      SetupAddElement(ps, iItemHolder );

      // tracks
      if (item->getConfig()->value<bool>("Draw Tracks"))
      {
         for(reco::Vertex::trackRef_iterator it = iData.tracks_begin() ;
             it != iData.tracks_end()  ; ++it)
         {
            float w = iData.trackWeight(*it);
            if (w < 0.5) continue;
      
            const reco::Track & track = *it->get();
            REveRecTrack t;
            t.fBeta = 1.;
            t.fV = REveVector(track.vx(), track.vy(), track.vz());
            t.fP = REveVector(track.px(), track.py(), track.pz());
            t.fSign = track.charge();
            REveTrack* trk = new REveTrack(&t, context->getTrackPropagator());
            trk->MakeTrack(); 
            SetupAddElement(trk, iItemHolder);
         }
      }
      if (item->getConfig()->value<bool>("Draw Pseudo Track")) {
         REveRecTrack t;
         t.fBeta = 1.;
         t.fV = REveVector(iData.x(), iData.y(), iData.z());
         t.fP = REveVector(-iData.p4().px(), -iData.p4().py(), -iData.p4().pz());
         t.fSign = 1;
         REveTrack* trk = new REveTrack(&t, context->getTrackPropagator());
         trk->SetLineStyle(7);
         trk->MakeTrack();
         SetupAddElement(trk, iItemHolder);
      }
   }
};


REGISTER_FW2PROXYBUILDER(FWVertexProxyBuilder, reco::Vertex, "Vertices");

#endif
