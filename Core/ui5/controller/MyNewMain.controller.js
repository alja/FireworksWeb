sap.ui.define(['rootui5/eve7/controller/Main.controller',
               'rootui5/eve7/lib/EveManager'
], function(MainController, EveManager) {
   "use strict";    


   
   return MainController.extend("custom.MyNewMain", {                    

      onInit: function() {
         // console.log('MAIN CONTROLLER INIT');
         MainController.prototype.onInit.apply(this, arguments);
         this.mgr.handle.SetReceiver(this);
         this.cnt = 0;


         
      },
      
      onManagerUpdate: function() {
         MainController.prototype.onManagerUpdate.apply(this, arguments);
         var world = this.mgr.childs[0].childs;

         // this is a prediction that the fireworks GUI is the last element after scenes
         // could loop all the elements in top level and check for typename
         var last = world.length -1;
         console.log("init gui ", last, world);
         
         if (world[last]._typename == "FW2GUI") {
            this.fw2gui = (world[last]);

            var pthis = this;
            this.mgr.UT_refresh_event_info = function() {               
               console.log("jay ", world[last]);
               pthis.showEventInfo();
            }

             pthis.showEventInfo();
         }
      },


      OnWebsocketMsg : function(handle, msg, offset)
      {
         if ( false && typeof msg == "string") {            
            console.log("test ", msg.substr(0,4));
            if ( msg.substr(0,4) == "FW2_") {
               var resp = JSON.parse(msg.substring(4));
               console.log("HARIBU ", resp);
               return;
            }
         }
         this.mgr.OnWebsocketMsg(handle, msg, offset);
      },

      showHelp : function(oEvent) {
         alert("User support: fireworks@cern.ch");
      },

      showEventInfo : function() {
         document.title = "Fireworks: " + this.fw2gui.fname + " " + this.fw2gui.eventCnt + "/" + this.fw2gui.size;
         this.byId("runInput").setValue(this.fw2gui.run);
         this.byId("lumiInput").setValue(this.fw2gui.lumi);
         this.byId("eventInput").setValue(this.fw2gui.event);
         
         this.byId("dateLabel").setText(this.fw2gui.date);
      },

      nextEvent : function(oEvent) {
         this.cnt++;
         this.mgr.SendMIR({ "mir":        "NextEvent()",
                            "fElementId": this.fw2gui.fElementId,
                            "class":      "FW2GUI"
                          });
      },
      
      prevEvent : function(oEvent) {
         this.cnt++;
         this.mgr.SendMIR({ "mir":        "PreviousEvent()",
                            "fElementId": this.fw2gui.fElementId,
                            "class":      "FW2GUI"
                          });
      }
   });
});
