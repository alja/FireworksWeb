sap.ui.define(['rootui5/eve7/controller/Main.controller',
               'rootui5/eve7/lib/EveManager'
], function(MainController, EveManager) {
   "use strict";



   return MainController.extend("custom.MyNewMain", {

      onInit: function() {
         console.log('MAIN CONTROLLER INIT 2');
         MainController.prototype.onInit.apply(this, arguments);
         this.mgr.handle.SetReceiver(this);
         this.mgr.RegisterController(this);
      },

      OnWebsocketClosed : function() {
         var elem = this.byId("CanvasMainPage");
         elem.setTitle("DISCONNECTED!!!!");
      },

      OnEveManagerInit: function() {
         MainController.prototype.OnEveManagerInit.apply(this, arguments);
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
         if ( typeof msg == "string") {
            console.log("test my maninController Add  collection ", msg.substr(0,40));
            if ( msg.substr(0,4) == "FW2_") {
               var resp = JSON.parse(msg.substring(4));
               var fnName = "addCollectionResponse";
               this[fnName](resp);
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
         this.mgr.SendMIR({ "mir":        "NextEvent()",
                            "fElementId": this.fw2gui.fElementId,
                            "class":      "FW2GUI"
                          });
      },

      prevEvent : function(oEvent) {
         this.mgr.SendMIR({ "mir":        "PreviousEvent()",
                            "fElementId": this.fw2gui.fElementId,
                            "class":      "FW2GUI"
                          });
      },

      addCollectionResponse: function(msg) {
         console.log("addCollectionResponse", msg.arr);
         if (this.addCollectionTable == null) {
            this.makeAddCollectionTable(msg)
         }
         this.addCollectionTable.open();
      },

     //==============================================================================
     //==============================================================================


       makeAddCollectionTable: function(msg) {

	 var oItemTemplate2 = new sap.m.ColumnListItem({
	    type : "Active",
	    unread : false,
	    cells : [
	       new sap.m.Label({
		  text : "{purpose}"
	       }), new sap.m.Label({
		  text: "{moduleLabel}"
	       }), new sap.m.Label({
		  text: "{processName}"
	       }), new sap.m.Label({
		  text : "{type}"
	       })
	    ]
	 });



	 var fnCreateColumsForDialog5 = function () {
	    return [
	       new sap.m.Column({
		  width : "70px",
		  hAlign : "Begin",
		  header : new sap.m.Label({
		     text : "Purpose"
		  })
	       }),
	       new sap.m.Column({
		  hAlign : "Begin",
		  width : "159px",
		  header : new sap.m.Label({
		     text : "ModuleLabel"
		  })
	       }),
	       new sap.m.Column({
		  hAlign : "Begin",
		  width : "70px",
		  header : new sap.m.Label({
		     text : "ProcessName"
		  })
	       }),
	       new sap.m.Column({
		  hAlign : "Center",
		  width : "200px",
		  header : new sap.m.Label({
		     text : "Type"
		  })
	       })
	    ];
	 };

         // filter function for the list search
	 var fnDoSearch = function (oEvent, bProductSearch) {
	    var aFilters = [],
		sSearchValue = oEvent.getParameter("value"),
		itemsBinding = oEvent.getParameter("itemsBinding");

	    // create the local filter to apply
	    if(sSearchValue !== undefined && sSearchValue.length > 0) {
	       if(bProductSearch) {
		  // create multi-field filter to allow search over all attributes
		  aFilters.push(new sap.ui.model.Filter("ProductId", sap.ui.model.FilterOperator.Contains , sSearchValue));
		  // apply the filter to the bound items, and the Select Dialog will update
		  itemsBinding.filter(aFilters, "Application");
	       } else {
		  // create multi-field filter to allow search over all attributes
		  aFilters.push(new sap.ui.model.Filter("purpose", sap.ui.model.FilterOperator.Contains , sSearchValue));
		  aFilters.push(new sap.ui.model.Filter("type", sap.ui.model.FilterOperator.Contains , sSearchValue));
		  aFilters.push(new sap.ui.model.Filter("moduleLabel", sap.ui.model.FilterOperator.Contains , sSearchValue));
		  aFilters.push(new sap.ui.model.Filter("Type", sap.ui.model.FilterOperator.Contains , sSearchValue));
		  // apply the filter to the bound items, and the Select Dialog will update
		  itemsBinding.filter(new sap.ui.model.Filter(aFilters, false), "Application"); // filters connected with OR
	       }
	    } else {
	       // filter with empty array to reset filters
	       itemsBinding.filter(aFilters, "Application");
	    }
	 };


	 // create the model to hold the data
	 var oModel2 = new sap.ui.model.json.JSONModel();
	 oModel2.setDefaultBindingMode("OneWay");
	 oModel2.setData(msg);



	 /* 5) multi select table dialog with large binding */

	    this.addCollectionTable = new sap.m.TableSelectDialog("TableSelectDialog5", {
	    title: "Choose EDM collection",
	    noDataText: "Sorry, no data",
	    multiSelect: true,
	    search : fnDoSearch,
	    hAlign : "Begin",
	    liveChange: fnDoSearch,
	    columns : [
	       fnCreateColumsForDialog5()
	    ]
	 });

          this.addCollectionTable.pmain = this;

	 // set model & bind Aggregation
	 this.addCollectionTable.setModel(oModel2);
	 this.addCollectionTable.bindAggregation("items", "/arr", oItemTemplate2);


	 // attach confirm listener
	 this.addCollectionTable.attachConfirm(function (evt) {
	    var aSelectedItems = evt.getParameter("selectedItems");
	    if (aSelectedItems) {
               var main = this.pmain;
               
	       //Loop through all selected items
	       for (var i=0; i<aSelectedItems.length; i++) {
		  //Get all the cells and pull back the first one which will be the name content
		  var oCells = aSelectedItems[i].getCells();
		  var oCell = oCells[0];
                  console.log("selected cell ", oCells);
                  var fcall = "AddCollection(\"" + oCells[0].getText() + "\", \"" + oCells[1].getText() + "\", \"" +  oCells[2].getText() + "\", \"" + oCells[3].getText() + "\")";
                  console.log("fcall MIR ", fcall);
                  main.mgr.SendMIR({ "mir":        fcall,
                            "fElementId": main.fw2gui.fElementId,
                            "class":      "FW2GUI"
                              });
                  return; // take only the first one 
	       }
	    }
	 });

      }

   });
});
