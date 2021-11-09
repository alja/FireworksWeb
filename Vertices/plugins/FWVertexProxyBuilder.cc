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

#include "FireworksWeb/Core/interface/FWEventItem.h"
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

   using REveDataSimpleProxyBuilderTemplate<reco::Vertex>::Build;

   virtual void Build(const reco::Vertex &iData, int iIndex, ROOT::Experimental::REveElement *iItemHolder, const ROOT::Experimental::REveViewContext *vc) override
   {
      //std::cout << "vertex error \n" << iData.error() << std::endl;
      //printf("position %g, %g, %g \n", iData.x(), iData.y(), iData.z());
      reco::Vertex::Error e = iData.error();

      bool showEllipse = false;
      if (showEllipse)
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

         // AMT TODO -- need to find a way to set the factor externally
         // original range [0, 10]
         float scale = 10;

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
      // AMT TODO ... this hould also be an external configuration
      //
      if (0)
      {
         auto context = fireworks::Context::getInstance();
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
   }
};


REGISTER_FW2PROXYBUILDER(FWVertexProxyBuilder, reco::Vertex, "Vertices");

#endif
