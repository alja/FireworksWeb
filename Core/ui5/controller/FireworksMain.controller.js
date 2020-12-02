sap.ui.define(['rootui5/eve7/controller/Main.controller',
               'rootui5/eve7/lib/EveManager'
], function(MainController, EveManager) {
   "use strict";
   return MainController.extend("fw.FireworksMain", {

      onInit: function() {
         console.log('MAIN CONTROLLER INIT 2');
         MainController.prototype.onInit.apply(this, arguments);
         this.mgr.handle.SetReceiver(this);
         this.mgr.RegisterController(this);
      },

      OnWebsocketClosed : function() {
         var elem = this.byId("centerTitle");
         elem.setHtmlText("<strong style=\"color: red;\">Client Disconnected !</strong>");
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
         this.mgr.SendMIR("NextEvent()", this.fw2gui.fElementId,  "FW2GUI");
      },

      prevEvent : function(oEvent) {
         this.mgr.SendMIR("PreviousEvent()", this.fw2gui.fElementId,  "FW2GUI");
      },

      addCollectionResponse: function(msg) {         
         console.log("addCollectionResponse", msg.arr);
         if (this.table == null) {
            this.makeAddCollection(msg.arr)
         }
         this.popover.openBy(this.byId("__xmlview0--Summary--addCollection"));
      },

     //==============================================================================
     //==============================================================================

      makeAddCollection: function (data){
         if (!this.table)
            this.createTable(data);

         if (!this.popover) {	    
            this.popover = new sap.m.Popover("popupTable", {title:"Add EDM Collection"});

	    
	    let sw = new sap.m.SearchField();
	    sw.placeholder="Filter";
	    var pt = this.table;
	    sw.attachSearch(function(oEvent) {
	       var txt = oEvent.getParameter("query");	       
	       let filter = new sap.ui.model.Filter([new sap.ui.model.Filter("purpose", sap.ui.model.FilterOperator.Contains, txt), new sap.ui.model.Filter("moduleLabel", sap.ui.model.FilterOperator.Contains, txt),new sap.ui.model.Filter("processName", sap.ui.model.FilterOperator.Contains, txt), new sap.ui.model.Filter("type", sap.ui.model.FilterOperator.Contains, txt)],false);
	       pt.getBinding("items").filter(filter, "Applications");
	    });
	    
            this.popover.addContent(sw);
            this.popover.addContent(this.table);

	    // footer
	    var pthis = this;
	    let fa = new sap.m.OverflowToolbar();
	    let b1 = new sap.m.Button({text:"AddCollection"});
	    fa.addContent(b1);
	    b1.attachPress(function(oEvent) {	       
               var oSelectedItem = pt.getSelectedItems(); 
	       var item1 = oSelectedItem[0];
	       console.log("SELECT ",item1.getBindingContext().getObject());
               var obj = item1.getBindingContext().getObject();
               var fcall = "AddCollection(\"" + obj.purpose + "\", \"" + obj.moduleLabel + "\", \"" + obj.processName + "\", \"" + obj.type + "\")";
               pthis.mgr.SendMIR(fcall, pthis.fw2gui.fElementId, "FW2GUI");
	    });
	    
	    let b2 = new sap.m.Button({text:"Close"});
	    fa.addContent(b2);
	    b2.attachPress(function(oEvent) {
	       pthis.popover.close();
	    });
	    this.popover.setFooter(fa);
         }
      },
      
      
      createTable: function(data) {
	 // create a Model with this data
	 var model = new sap.ui.model.json.JSONModel();
	 model.setData(data);


	 // create the UI

	 // create a sap.m.Table control
	 var table = new sap.m.Table("tableTest",{
	    mode:"SingleSelect",
	    columns: [
	       new sap.m.Column("purpose", {header: new sap.m.Text({text: "Purpose"})}),
	       new sap.m.Column("moduleLabel", {header: new sap.m.Text({text:"ModuleLabel"})}),
	       new sap.m.Column("processName", {header: new sap.m.Text({text: "ProcessName"})}),
	       new sap.m.Column("type", {header: new sap.m.Text({text:"Type"})})
	    ]
	 });
	 table.setIncludeItemInSelection(true);
         this.table = table;
	 table.bActiveHeaders = true;

	 table.attachEvent("columnPress", function(evt) {

            var col = evt.getParameters().column;
	    var sv = false;

	    // init first time ascend
	    if (col.getSortIndicator() == sap.ui.core.SortOrder.Descend || col.getSortIndicator() == sap.ui.core.SortOrder.None ) {
	       sv = true;
	    }
	    else {
               sv = false;
	    }
	    
	    var oSorter = new sap.ui.model.Sorter(col.sId, sv);	    
	    var oItems = this.getBinding("items");
	    oItems.sort(oSorter);

	    var indicator = sv ?  sap.ui.core.SortOrder.Descending :  sap.ui.core.SortOrder.Ascending;
	    col.setSortIndicator(indicator);
	 });


	 // bind the Table items to the data collection
	 table.bindItems({
	    path : "/",				  
	    template : new sap.m.ColumnListItem({
	       cells: [
		  new sap.m.Text({text: "{purpose}"}),
		  new sap.m.Text({text: "{moduleLabel}"}),
		  new sap.m.Text({text: "{processName}"}),
		  new sap.m.Text({text: "{type}"})
	       ]
	    })
	 });
	 // set the model to the Table, so it knows which data to use
	 table.setModel(model);
      }
   });
});
