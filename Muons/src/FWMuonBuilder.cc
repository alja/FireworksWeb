// -*- C++ -*-
//
// Package:     Muons
// Class  :     FWMuonBuilder
//

#include "ROOT/REveVSDStructs.hxx"
#include "ROOT/REveTrack.hxx"
#include "ROOT/REveStraightLineSet.hxx"
#include "ROOT/REveDataProxyBuilderBase.hxx"
#include "ROOT/REveGeoShape.hxx"
// #include "ROOT/REveGeoNode.hxx" ???
#include "TGeoArb8.h"

#include "Fireworks2/Core/interface/FWMagField.h"
#include "Fireworks2/Core/interface/FWGeometry.h"
#include "Fireworks2/Core/interface/Context.h"
#include "Fireworks2/Core/interface/fwLog.h"

#include "Fireworks2/Candidates/interface/CandidateUtils.h"

#include "Fireworks2/Tracks/interface/TrackUtils.h"
#include "Fireworks2/Tracks/interface/estimate_field.h"

#include "Fireworks2/Muons/interface/FWMuonBuilder.h"
#include "Fireworks2/Muons/interface/SegmentUtils.h"

#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/MuonDetId/interface/MuonSubdetId.h"

using namespace ROOT::Experimental;

namespace  {

std::vector<REveVector> getRecoTrajectoryPoints( const reco::Muon* muon)
{
   std::vector<REveVector> points;
   
   float localTrajectoryPoint[3];
   float globalTrajectoryPoint[3];
   
   const FWGeometry* geom = fireworks::Context::getInstance()->getGeom();
  
   const std::vector<reco::MuonChamberMatch>& matches = muon->matches();
   for( std::vector<reco::MuonChamberMatch>::const_iterator chamber = matches.begin(),
							 chamberEnd = matches.end();
	chamber != chamberEnd; ++chamber )
   {
      // expected track position
      localTrajectoryPoint[0] = chamber->x;
      localTrajectoryPoint[1] = chamber->y;
      localTrajectoryPoint[2] = 0;

      unsigned int rawid = chamber->id.rawId();
      if( geom->contains( rawid ))
      {
	 geom->localToGlobal( rawid, localTrajectoryPoint, globalTrajectoryPoint );
	 points.push_back( REveVector(globalTrajectoryPoint[0],
				      globalTrajectoryPoint[1],
				      globalTrajectoryPoint[2] ));
      }
   }
   return points;
}

//______________________________________________________________________________

void addMatchInformation( const reco::Muon* muon,
                          REveDataProxyBuilderBase* pb,
                          REveElement* parentList,
                          bool showEndcap )
{
  std::set<unsigned int> ids;
  const FWGeometry* geom = fireworks::Context::getInstance()->getGeom();
  
  const std::vector<reco::MuonChamberMatch>& matches = muon->matches();
   
  //need to use auto_ptr since the segmentSet may not be passed to muonList
  std::unique_ptr<REveStraightLineSet> segmentSet( new REveStraightLineSet );
  // FIXME: This should be set elsewhere.
  segmentSet->SetLineWidth( 4 );

  for( std::vector<reco::MuonChamberMatch>::const_iterator chamber = matches.begin(), 
						       chambersEnd = matches.end(); 
       chamber != chambersEnd; ++chamber )
  {
    unsigned int rawid = chamber->id.rawId();
    float segmentLength = 0.0;
    float segmentLimit  = 0.0;

    if( geom->contains( rawid ))
    {
      REveGeoShape* shape = geom->getEveShape( rawid );
      shape->SetName( "Chamber" );
      shape->RefMainTrans().Scale( 0.999, 0.999, 0.999 );

      FWGeometry::IdToInfoItr det = geom->find( rawid );
      if( det->shape[0] == 1 ) // TGeoTrap
      {
        segmentLength = det->shape[3];
        segmentLimit  = det->shape[4];
      }
      else if( det->shape[0] == 2 ) // TGeoBBox
      {
	segmentLength = det->shape[3];
      }
      else
      {   
        const double segmentLength = 15;
        fwLog( fwlog::kWarning ) << Form("FWMuonBuilder: unknown shape type in muon chamber with detId=%d. Setting segment length to %.0f cm.\n",  rawid, segmentLength);
      }
        
      if( ids.insert( rawid ).second &&  // ensure that we add same chamber only once
	  ( chamber->detector() != MuonSubdetId::CSC || showEndcap ))
      {     
	pb->SetupAddElement( shape, parentList );
      }
     
      for( std::vector<reco::MuonSegmentMatch>::const_iterator segment = chamber->segmentMatches.begin(),
							    segmentEnd = chamber->segmentMatches.end();
	   segment != segmentEnd; ++segment )
      {
	float segmentPosition[3]  = {    segment->x,     segment->y, 0.0 };
	float segmentDirection[3] = { segment->dXdZ,  segment->dYdZ, 0.0 };

	float localSegmentInnerPoint[3];
	float localSegmentOuterPoint[3];

	fireworks::createSegment( chamber->detector(), true, 
				  segmentLength, segmentLimit, 
				  segmentPosition, segmentDirection,
				  localSegmentInnerPoint, localSegmentOuterPoint );
      
	float globalSegmentInnerPoint[3];
	float globalSegmentOuterPoint[3];

	geom->localToGlobal( *det, localSegmentInnerPoint,  globalSegmentInnerPoint );
	geom->localToGlobal( *det, localSegmentOuterPoint,  globalSegmentOuterPoint );

	segmentSet->AddLine( globalSegmentInnerPoint[0], globalSegmentInnerPoint[1], globalSegmentInnerPoint[2],
			     globalSegmentOuterPoint[0], globalSegmentOuterPoint[1], globalSegmentOuterPoint[2] );
      }
    }
  }
  
  if( !matches.empty() ) 
    pb->SetupAddElement( segmentSet.release(), parentList );
}

//______________________________________________________________________________

bool
buggyMuon( const reco::Muon* muon,
           const FWGeometry* geom )
{
   if( !muon->standAloneMuon().isAvailable() ||
       !muon->standAloneMuon()->extra().isAvailable())
     return false;
   
   float localTrajectoryPoint[3];
   float globalTrajectoryPoint[3];
   
   const std::vector<reco::MuonChamberMatch>& matches = muon->matches();
   for( std::vector<reco::MuonChamberMatch>::const_iterator chamber = matches.begin(),
							 chamberEnd = matches.end();
	chamber != chamberEnd; ++chamber )
   {
      // expected track position
      localTrajectoryPoint[0] = chamber->x;
      localTrajectoryPoint[1] = chamber->y;
      localTrajectoryPoint[2] = 0;

      unsigned int rawid = chamber->id.rawId();
      if( geom->contains( rawid ))
      {
	 geom->localToGlobal( rawid, localTrajectoryPoint, globalTrajectoryPoint );
         double phi = atan2( globalTrajectoryPoint[1], globalTrajectoryPoint[0] );
         if( cos( phi - muon->standAloneMuon()->innerPosition().phi()) < 0 )
            return true;
      }
   }
   return false;
}

REveTrack* prepareMuonTrackWithExtraPoints(const reco::Track& track,
              REveTrackPropagator* propagator,
              const std::vector<REveVector>& extraPoints )
{
   REveRecTrack t;
   t.fBeta = 1.;
   t.fSign = track.charge();
   t.fV.Set(track.vx(), track.vy(), track.vz());
   t.fP.Set(track.px(), track.py(), track.pz());
   //  t.fSign = muon->charge();
   //  t.fV.Set(muon->vx(), muon->vy(), muon->vz());
   //  t.fP.Set(muon->px(), muon->py(), muon->pz());
   REveTrack* trk = new REveTrack( &t, propagator );
   size_t n = extraPoints.size();

   if (n > 1) {
      int lastDaughter = n-2;
      for (int i = 0; i <= lastDaughter; ++i)
         trk->AddPathMark( REvePathMark( REvePathMark::kDaughter, extraPoints[i] ) );
   }
   trk->AddPathMark( REvePathMark( REvePathMark::kDecay, extraPoints.back() ) );
   return trk;
}


}

//
// constructors and destructor
//
FWMuonBuilder::FWMuonBuilder():m_lineWidth(2)
{
}

FWMuonBuilder::~FWMuonBuilder()
{
}

//
// member functions
//
//______________________________________________________________________________

void
FWMuonBuilder::calculateField( const reco::Muon& iData, FWMagField* field )
{
   // if auto field estimation mode, do extra loop over muons.
   // use both inner and outer track if available
   if( field->getSource() == FWMagField::kNone )
   {
      if( fabs( iData.eta() ) > 2.0 || iData.pt() < 3 ) return;
      if( iData.innerTrack().isAvailable())
      {
         double estimate = fw::estimate_field( *( iData.innerTrack()), true );
         if( estimate >= 0 ) field->guessField( estimate );	 
      }
      if( iData.outerTrack().isAvailable() )
      {
         double estimate = fw::estimate_field( *( iData.outerTrack()));
         if( estimate >= 0 ) field->guessFieldIsOn( estimate > 0.5 );
      }
   }
}

//______________________________________________________________________________

void
FWMuonBuilder::buildMuon( REveDataProxyBuilderBase* pb,
			  const reco::Muon* muon,
			  REveElement* tList,
			  bool showEndcap,
			  bool tracksOnly )
{
   fireworks::Context* context = fireworks::Context::getInstance();
   std::cout << "rwet ctx " << context->caloMaxEta() << std::endl;
   std::cout << "rwet field " << context->getField() << std::endl;
   std::cout << "rwet field 2" << context->getField()->getSource() << std::endl;
   calculateField( *muon, context->getField());
  
   REveRecTrack recTrack;
   recTrack.fBeta = 1.;

   // If we deal with a tracker muon we use the inner track and guide it
   // through the trajectory points from the reconstruction. Segments
   // represent hits. Matching between hits and the trajectory shows
   // how well the inner track matches with the muon hypothesis.
   //
   // In other cases we use a global muon track with a few states from 
   // the inner and outer tracks or just the outer track if it's the
   // only option

   if( muon->isTrackerMuon() && 
       muon->innerTrack().isAvailable() &&
       muon->isMatchesValid() &&
       !buggyMuon( &*muon, context->getGeom()))
   {
      REveTrack* trk = fireworks::prepareTrack( *(muon->innerTrack()),
						context->getMuonTrackPropagator(),
						getRecoTrajectoryPoints( muon));
      trk->MakeTrack();
      trk->SetLineWidth(m_lineWidth);
      pb->SetupAddElement( trk, tList );
      if( ! tracksOnly )
	 addMatchInformation( &(*muon), pb, tList, showEndcap );
      return;
   } 

   if( muon->isGlobalMuon() &&
       muon->globalTrack().isAvailable())
   {
      std::vector<REveVector> extraPoints;
      if( muon->innerTrack().isAvailable() &&  muon->innerTrack()->extra().isAvailable())
      {
	 extraPoints.push_back( REveVector( muon->innerTrack()->innerPosition().x(),
					    muon->innerTrack()->innerPosition().y(),
					    muon->innerTrack()->innerPosition().z()));
	 extraPoints.push_back( REveVector( muon->innerTrack()->outerPosition().x(),
					    muon->innerTrack()->outerPosition().y(),
					    muon->innerTrack()->outerPosition().z()));
      }
      if( muon->outerTrack().isAvailable() &&  muon->outerTrack()->extra().isAvailable())
      {
	 extraPoints.push_back( REveVector( muon->outerTrack()->innerPosition().x(),
					    muon->outerTrack()->innerPosition().y(),
					    muon->outerTrack()->innerPosition().z()));
	 extraPoints.push_back( REveVector( muon->outerTrack()->outerPosition().x(),
					    muon->outerTrack()->outerPosition().y(),
					    muon->outerTrack()->outerPosition().z()));
      }
      REveTrack* trk = nullptr;
      if (extraPoints.empty())
         trk = fireworks::prepareTrack( *( muon->globalTrack()),context->getMuonTrackPropagator());
      else
         trk = prepareMuonTrackWithExtraPoints(*( muon->globalTrack()),context->getMuonTrackPropagator(), extraPoints); 

      trk->MakeTrack();
      trk->SetLineWidth(m_lineWidth);
      pb->SetupAddElement( trk, tList );
      return;
   }

   if( muon->innerTrack().isAvailable())
   {
      REveTrack* trk = fireworks::prepareTrack( *( muon->innerTrack()), context->getMuonTrackPropagator());
      trk->MakeTrack();
      pb->SetupAddElement( trk, tList );
      return;
   }

   if( muon->outerTrack().isAvailable())
   {
      REveTrack* trk = fireworks::prepareTrack( *( muon->outerTrack()), context->getMuonTrackPropagator());
      trk->MakeTrack();
      trk->SetLineWidth(m_lineWidth);
      pb->SetupAddElement( trk, tList );
      return;
   }
   
   // if got that far it means we have nothing but a candidate
   // show it anyway.
   REveTrack* trk = fireworks::prepareCandidate( *muon, context->getMuonTrackPropagator());
   trk->MakeTrack();
   trk->SetLineWidth(m_lineWidth);
   pb->SetupAddElement( trk, tList );
}
