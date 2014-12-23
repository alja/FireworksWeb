{
   printf("\n\n\n Going to call TTree draw ###########################################################\n");
   // apply filter
   TEventList* flist = 0;
   flist = new TEventList("fworks_filter");
   //   printf("Filter selected %d events before\n", flist->GetN());
   // eventTree->Draw(">>fworks_filter", "recoTracks_generalTracks__reRECO..obj.pt() > 5");
   eventTree->Draw(">>fworks_filter", "sqrt(recoTracks_generalTracks__reRECO.obj.momentum_.fCoordinates.fX*recoTracks_generalTracks__reRECO.obj.momentum_.fCoordinates.fX + recoTracks_generalTracks__reRECO.obj.momentum_.fCoordinates.fY*recoTracks_generalTracks__reRECO.obj.momentum_.fCoordinates.fY) > 225");
   printf("Filter selected %d events\n", flist->GetN());


   for (Long64_t e = 0; e < 3; ++e)
   {
      printf("========================================\n");
      printf("Event %lld:\n", e);

      // go to given event
      event->to(e);
      eventTree->LoadTree(e);


      // print first few tracks
      edm::Handle<reco::TrackCollection> handle_tracks;
      edm::InputTag tag("generalTracks");
      try
      {
         event->getByLabel(tag, handle_tracks);
   
         const reco::TrackCollection *tracks = &*handle_tracks;
         printf("general tracks %p \n", (void*)tracks);
   
         for (int i = 0; i < 2; ++i)
            printf("tracks [%d] pt = %.3f\n", i, tracks->at(i).pt());
      }
      catch (const cms::Exception& iE)
      {
         std::cerr << iE.what() <<std::endl;
         return 0;
      }
   }

   return 0;
}
