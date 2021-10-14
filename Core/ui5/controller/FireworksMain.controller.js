sap.ui.define(['rootui5/eve7/controller/Main.controller',
   'rootui5/eve7/lib/EveManager',
   "sap/ui/core/mvc/XMLView",
   "sap/ui/core/util/File",
   "sap/m/MessageBox"
], function (MainController, EveManager, XMLView, File, MessageBox) {
   "use strict";
   return MainController.extend("fw.FireworksMain", {

      onInit: function () {
         MainController.prototype.onInit.apply(this, arguments);
         this.mgr.handle.setReceiver(this);
         this.mgr.RegisterController(this);
         // var elem = this.byId("centerTitle");
         //  elem.setHtmlText("<strong> CMS Web Event Display </strong>");
      },

      onWebsocketClosed: function () {
         var elem = this.byId("centerTitle");
         elem.setHtmlText("<strong style=\"color: red;\">Client Disconnected !</strong>");
      },

      onEveManagerInit: function () {
         MainController.prototype.onEveManagerInit.apply(this, arguments);
         var world = this.mgr.childs[0].childs;

         // this is a prediction that the fireworks GUI is the last element after scenes
         // could loop all the elements in top level and check for typename
         var last = world.length - 1;
         console.log("init gui ", last, world);

         if (world[last]._typename == "FW2GUI") {
            this.fw2gui = (world[last]);

            var pthis = this;
            this.mgr.UT_refresh_event_info = function () {
               pthis.showEventInfo();
            }

            pthis.showEventInfo();


            this.mgr.UT_refresh_filter_info = function () {
               console.log("AMT UT_refresh_filter_info going to refresh filter");
               pthis.refreshFilterInfo();

            }
            console.log("onEveManagerInit ", this.fw2gui.childs[0].enabled);
            if (this.fw2gui.childs[0].enabled)
               this.byId("enableFilter").setSelected(true);

         }
      },

      onWebsocketMsg: function (handle, msg, offset) {
         if (typeof msg == "string") {
            if (msg.substr(0, 4) == "FW2_") {
               var resp = JSON.parse(msg.substring(4));
               var fnName = resp["action"];
               this[fnName](resp);
               return;
            }
         }
         this.mgr.onWebsocketMsg(handle, msg, offset);
      },

      saveConfiguration: function (oEvent) {
         this.mgr.SendMIR("requestConfiguration()", this.fw2gui.fElementId, "FW2GUI");
      },

      saveConfigurationResponse: function (cfg) {
         console.log("Going to Save configuration \n", cfg.body);
         File.save(cfg.body, "fireworks", "fwc", "application/xml");
      },
      showCmsInfo: function () {
         
         let x = JSROOT.source_dir.split("/");
         let cv =  x[x.length-3];
         // console.log("startup ", x);
         MessageBox.information("Server DataFormat: CMSSW_11_3_0\nFireworks Client: " + cv);
      },
      showMailAddress: function (oEvent) {
         MessageBox.information("Mail to: \nfireworks-support@cern.ch");
      },
      userGuide: function (oEvent) {
         sap.m.URLHelper.redirect("https://github.com/alja/FireworksWeb/blob/july/doc/UserGuide.md#table-of-contents", true);
      },
      troubleshoot: function (oEvent) {
         sap.m.URLHelper.redirect("https://github.com/alja/FireworksWeb/blob/july/doc/TroubleShooting.md", true);
      },

      showEventInfo: function () {
         document.title = this.fw2gui.title;
         this.byId("runInput").setValue(this.fw2gui.run);
         this.byId("lumiInput").setValue(this.fw2gui.lumi);
         this.byId("eventInput").setValue(this.fw2gui.event);

         
         let ifp = this.fw2gui.title.match(/(.*) \[(.*)\]$/);
         let path = ifp[1];
         this.byId("fileName").setText(path);
         this.byId("fileName").setTooltip(path);
         this.byId("fileName").setDesign("Bold");

         this.byId("fileNav").setText(ifp[2]);
         this.byId("fileNav").setDesign("Bold");

         this.byId("autoplayId").setSelected(this.fw2gui.autoplay);
         //this.byId("playdelayId").setValue(this.fw2gui.playdelay);
      

      },

      refreshFilterInfo: function () {
         let s = this.fw2gui.childs[0].statusID == 1 ? true : false;
         console.log("main controller .... test filter selected =  ", s);
         this.byId("enableFilter").setSelected(s);
         if (this.eventFilter) {
            this.eventFilter.refreshEveFilterResults(this.fw2gui.childs[0]);
         }
      },
      firstEvent: function (oEvent) {
         this.mgr.SendMIR("FirstEvent()", this.fw2gui.fElementId, "FW2GUI");
      },
      lastEvent: function (oEvent) {
         this.mgr.SendMIR("LastEvent()", this.fw2gui.fElementId, "FW2GUI");
      },
      nextEvent: function (oEvent) {
         this.mgr.SendMIR("NextEvent()", this.fw2gui.fElementId, "FW2GUI");
      },

      prevEvent: function (oEvent) {
         this.mgr.SendMIR("PreviousEvent()", this.fw2gui.fElementId, "FW2GUI");
      },

      goToEvent: function (oEvent) {
         console.log("goto event run = ", this.byId("runInput").getValue());
         let cmd = "goToRunEvent(" + this.byId("runInput").getValue() + ", " + this.byId("lumiInput").getValue() + ", " + this.byId("eventInput").getValue() + ")";
         this.mgr.SendMIR(cmd, this.fw2gui.fElementId, "FW2GUI");
      },

      autoplay: function (oEvent) {
         console.log("AUTO", oEvent.getParameter("selected"));
         this.mgr.SendMIR("autoplay(" + oEvent.getParameter("selected") + ")", this.fw2gui.fElementId, "FW2GUI");
      },

      playdelay: function (oEvent) {
         console.log("playdelay ", oEvent.getParameters());
         this.mgr.SendMIR("playdelay(" + oEvent.getParameter("value") + ")", this.fw2gui.fElementId, "FW2GUI");
      },

      addCollectionResponse: function (msg) {
         console.log("addCollectionResponse", msg.arr);
         if (this.table == null) {
            this.makeAddCollection(msg.arr)
         }
         this.popover.openBy(this.byId("__xmlview0--Summary--addCollection"));
      },

      enableFilter: function (oEvent) {
         console.log("enable filter", oEvent.getParameter("selected"));
         let cmd = "toggleFilterEnabled(" + oEvent.getParameter("selected") + ")";
         this.mgr.SendMIR(cmd, this.fw2gui.childs[0].fElementId, "FWWebGUIEventFilter");
      },

      eventFilterShow: function () {

         if (this.eventFilter) {
            this.eventFilter.openFilterDialog();
         }
         else {
            let pthis = this;
            XMLView.create({
               viewName: "fw.view.EventFilter",
               viewData: { "mgr": this.mgr, "gui": this.fw2gui.childs[0] }
            }).then(function (oView) {
               pthis.eventFilter = oView.getController();
               pthis.eventFilter.buildFilterGUI();
               pthis.eventFilter.openFilterDialog();
            });
         }
      },
      //==============================================================================
      //==============================================================================

      makeAddCollection: function (data) {
         if (!this.table)
            this.createTable(data);

         if (!this.popover) {
            this.popover = new sap.m.Popover("popupTable", { title: "Add EDM Collection" });


            let sw = new sap.m.SearchField();
            sw.placeholder = "Filter";
            var pt = this.table;
            sw.attachSearch(function (oEvent) {
               var txt = oEvent.getParameter("query");
               let filter = new sap.ui.model.Filter([new sap.ui.model.Filter("purpose", sap.ui.model.FilterOperator.Contains, txt), new sap.ui.model.Filter("moduleLabel", sap.ui.model.FilterOperator.Contains, txt), new sap.ui.model.Filter("processName", sap.ui.model.FilterOperator.Contains, txt), new sap.ui.model.Filter("type", sap.ui.model.FilterOperator.Contains, txt)], false);
               pt.getBinding("items").filter(filter, "Applications");
            });

            this.popover.addContent(sw);
            this.popover.addContent(this.table);

            // footer
            var pthis = this;
            let fa = new sap.m.OverflowToolbar();
            let b1 = new sap.m.Button({ text: "AddCollection" });
            fa.addContent(b1);
            b1.attachPress(function (oEvent) {
               var oSelectedItem = pt.getSelectedItems();
               var item1 = oSelectedItem[0];
               console.log("SELECT ", item1.getBindingContext().getObject());
               var obj = item1.getBindingContext().getObject();
               var fcall = "AddCollection(\"" + obj.purpose + "\", \"" + obj.moduleLabel + "\", \"" + obj.processName + "\", \"" + obj.type + "\")";
               pthis.mgr.SendMIR(fcall, pthis.fw2gui.fElementId, "FW2GUI");
            });

            let b2 = new sap.m.Button({ text: "Close" });
            fa.addContent(b2);
            b2.attachPress(function (oEvent) {
               pthis.popover.close();
            });
            this.popover.setFooter(fa);
         }
      },


      createTable: function (data) {
         // create a Model with this data
         var model = new sap.ui.model.json.JSONModel();
         model.setData(data);


         // create the UI

         // create a sap.m.Table control
         var table = new sap.m.Table("tableTest", {
            mode: "SingleSelect",
            columns: [
               new sap.m.Column("purpose", { header: new sap.m.Text({ text: "Purpose" }) }),
               new sap.m.Column("moduleLabel", { header: new sap.m.Text({ text: "ModuleLabel" }) }),
               new sap.m.Column("processName", { header: new sap.m.Text({ text: "ProcessName" }) }),
               new sap.m.Column("type", { header: new sap.m.Text({ text: "Type" }) })
            ]
         });
         table.setIncludeItemInSelection(true);
         this.table = table;
         table.bActiveHeaders = true;

         table.attachEvent("columnPress", function (evt) {

            var col = evt.getParameters().column;
            var sv = false;

            // init first time ascend
            if (col.getSortIndicator() == sap.ui.core.SortOrder.Descend || col.getSortIndicator() == sap.ui.core.SortOrder.None) {
               sv = true;
            }
            else {
               sv = false;
            }

            var oSorter = new sap.ui.model.Sorter(col.sId, sv);
            var oItems = this.getBinding("items");
            oItems.sort(oSorter);

            var indicator = sv ? sap.ui.core.SortOrder.Descending : sap.ui.core.SortOrder.Ascending;
            col.setSortIndicator(indicator);
         });


         // bind the Table items to the data collection
         table.bindItems({
            path: "/",
            template: new sap.m.ColumnListItem({
               cells: [
                  new sap.m.Text({ text: "{purpose}" }),
                  new sap.m.Text({ text: "{moduleLabel}" }),
                  new sap.m.Text({ text: "{processName}" }),
                  new sap.m.Text({ text: "{type}" })
               ]
            })
         });
         // set the model to the Table, so it knows which data to use
         table.setModel(model);
      }
   });
});
