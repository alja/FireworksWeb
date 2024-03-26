sap.ui.define(['rootui5/eve7/controller/Main.controller',
   'rootui5/eve7/lib/EveManager',
   'sap/ui/core/Component',
   'sap/ui/core/UIComponent',
   "sap/ui/core/mvc/XMLView",
   "sap/ui/core/Fragment",
   "sap/ui/core/util/File",
   'rootui5/browser/controller/FileDialog.controller',
   'sap/ui/layout/Splitter',
   'sap/ui/layout/SplitterLayoutData',
   "sap/m/MessageBox",
   'sap/m/MenuItem'
], function (MainController, EveManager, Component, UIComponent, XMLView, Fragment, File, FileDialogController, Splitter, SplitterLayoutData, MessageBox, mMenuItem) {
   "use strict";
   return MainController.extend("fw.FireworksMain", {

      onInit: function () {
         MainController.prototype.onInit.apply(this, arguments);
         this.mgr.handle.setReceiver(this);
      },

      onWebsocketClosed: function () {
         var elem = this.byId("centerTitle");
         elem.setHtmlText("<strong style=\"color: red;\">Client Disconnected !</strong>");

         let tb = this.byId("otb1");

         tb.getContentLeft().forEach(function (item) {
            console.log(item);
            item.setEnabled(false);
         });
         tb.getContentMiddle().forEach(function (item) {
            console.log(item);
            if (item.setEnabled)
               item.setEnabled(false);
         });
         tb.getContentRight().forEach(function (item) {
            console.log(item);
            if (item.setEnabled)
               item.setEnabled(false);
         });

         let bb = this.byId("otb2");

         bb.getContent().forEach(function (item) {
            console.log(item);
            if (item.setEnabled)
               item.setEnabled(false);
         });
 
         let logBtn = this.byId("logButton");
         logBtn.setEnabled(true);
      },

      makeEveViewController: function(elem)
      {
         if (!elem.fRnrSelf)
             return;

         let viewid = "EveViewer" + elem.fElementId;
         let main = this;
         // create missing view
         console.log("Creating view", viewid);

         let vtype = "rootui5.eve7.view.GL";
         if (elem.fName === "Table")
            vtype = "rootui5.eve7.view.EveTable"; // AMT temporary solution
         else if (elem.fName === "Lego")
            vtype = "rootui5.eve7.view.Lego"; // AMT temporary solution
         else if (elem.fName === "TriggerTable")
                  vtype = "fw.view.TriggerTable"; // AMT temporary solution

         let oOwnerComponent = Component.getOwnerComponentFor(this.getView());
         let view = oOwnerComponent.runAsOwner(function() {
            return new sap.ui.xmlview({
               id: viewid,
               viewName: vtype,
               viewData: { mgr: main.mgr, eveViewerId: elem.fElementId },
            });
         });

         if (elem.fRnrSelf) {
            if (this.primarySplitter.getContentAreas().length == 1) {
               this.primarySplitter.addContentArea(view);
            }
            else {
               if (!this.primarySplitter.secondary) {
                  let vv = new Splitter("SecondaryViewSplitter", { orientation: "Vertical" });
                  vv.setLayoutData(new SplitterLayoutData({ resizable: true, size: "25%" }));
                  this.primarySplitter.addContentArea(vv);
                  this.primarySplitter.secondary = vv;
               }
               this.primarySplitter.secondary.addContentArea(view);
               this.setToolbarSwapIcon(view, "arrow-left");
            }
         }
         elem.ca = view;
         this.addInfoController(view, elem);
         // reset flag needed by UT_PostStream callback
         delete elem.pendInstance;
      },
      

      setToolbarExpandedAction(va) {
         let eveView = this.mgr.GetElement(va.eveViewerId);
         let bar = va.byId("tbar");
         let ca = bar.getContent();
         while (bar.getContent().length > 1)
            bar.removeContent(bar.getContent().length - 1);

         var bb = new sap.m.Button({
            type: sap.m.ButtonType.Default,
            text: "Back",
            enabled: true,
            press: function () {
               window.history.go(-1)
            }
         });
         bar.addContent(bb);
         let pthisF = this;

         var bb = new sap.m.Button({
            type: sap.m.ButtonType.Default,
            icon: "sap-icon://information",
            enabled: true,
            press: function () {
               ///window.history.go(-1)
               console.log("Open view controller");
               let vtype = "fw.view.3DViewController";

               if (eveView.fName == "RPhi" || eveView.fName == "RhoZ")
                  vtype = "fw.view.RPZViewController";
               else if (eveView.fName == "Table" || eveView.fName == "TriggerTable")
                  vtype = "fw.view.TableViewController";


               XMLView.create({
                  viewName: vtype,
                  viewData: { "mgr": pthisF.mgr, "eveView": eveView, "fw2gui": pthisF.fw2gui }
               }).then(function (oView) {
               });

            }
         });
         bar.addContent(bb);

      },

      setToolbarSwapIcon(va, iName)
      {
         let t = va.byId("tbar");
         let sBtn = t.getContent()[3];
         sBtn.setIcon("sap-icon://" + iName);
      },
      addInfoController: function (ui5view, eveView)
      {
         let bar = eveView.ca.byId("tbar");
         let pthisF = this;

         var bb = new sap.m.Button({
            type: sap.m.ButtonType.Default,
            icon:"sap-icon://message-information",
            enabled: true,
            press: function () {
               ///window.history.go(-1)
               console.log("Open view controller");
               let vtype = "fw.view.3DViewController";

               if (eveView.fName == "RPhi" || eveView.fName == "RhoZ")
                  vtype = "fw.view.RPZViewController";
               else if (eveView.fName == "Table" || eveView.fName == "TriggerTable")
                  vtype = "fw.view.TableViewController";
                  

               XMLView.create({
                  viewName: vtype,
                  viewData: { "mgr": pthisF.mgr, "eveView" : eveView, "fw2gui": pthisF.fw2gui }
               }).then(function (oView) {
               });
               
            }
         });
         bar.insertContent(bb, 1);
      },

      onEveManagerInit: function () {
         MainController.prototype.onEveManagerInit.apply(this, arguments);
         var world = this.mgr.childs[0].childs;

         // this is a prediction that the fireworks GUI is the last element after scenes
         // could loop all the elements in top level and check for typename
         var last = world.length - 1;
         // console.log("init gui ", last, world);

         if (world[last]._typename == "FW2GUI") {
            this.fw2gui = (world[last]);

            var pthis = this;
            this.mgr.UT_refresh_event_info = function () {
               pthis.showEventInfo();
            }

            pthis.showEventInfo();

            this.mgr.UT_refresh_filter_info = function () {
               pthis.refreshFilterInfo();

            }

            this.mgr.UT_refresh_invmass_dialog = function () {
               pthis.invMassDialogRefresh();
            }
             

            let filterEnabled = this.fw2gui.childs[0].statusID == 1 ? true : false;
            console.log("onEveManagerInit ", filterEnabled);
            this.byId("enableFilter").setSelected(filterEnabled);
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

      downloadConfiguration: function (oEvent) {
         this.mgr.SendMIR("requestConfiguration()", this.fw2gui.fElementId, "FW2GUI");
      },

      saveConfigurationResponse: function (cfg) {
         console.log("Going to Save configuration \n", cfg.body);
         File.save(cfg.body, "fireworks", "fwc", "application/xml");
      },

      /** @brief Invoke dialog with server side code */
      saveConfigurationToFileOnServer: function (tab) {
         // this.amtfn = "";
         let idx =  this.fw2gui.childs.length - 1;
         console.log("this.fw2gui.", this.fw2gui.childs[idx]);
         let fwp = "/Files system/" + this.fw2gui.childs[idx].fName;
         FileDialogController.SaveAs({
            websocket: this.mgr.handle,
            filename: "fireworks.fwc",
            can_change_path: this.fw2gui.standalone,
            working_path: fwp,
            title: "Select file name to configuration",
            filter: "Any files",
            filters: ["Config files (*.fwc)", "C++ files (*.cxx *.cpp *.c)", "Any files (*)"],
            onOk: fname => {
               let p = Math.max(fname.lastIndexOf("/"), fname.lastIndexOf("\\"));
               let title = (p > 0) ? fname.substr(p + 1) : fname;
               //this.amtfn = fname;
               let cmd = "saveConfigurationAs(\"" + fname + "\")";
               this.mgr.SendMIR(cmd, this.fw2gui.fElementId, "FW2GUI");
            },
            onCancel:  function () { console.log("FileDialoController OK"); },
            onFailure: function () { console.log("FileDialoController fail"); }
         });
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
         sap.m.URLHelper.redirect("https://github.com/alja/FireworksWeb/blob/master/doc/UserGuide.md#table-of-contents", true);
      },
      troubleshoot: function (oEvent) {
         sap.m.URLHelper.redirect("https://github.com/alja/FireworksWeb/blob/master/doc/TroubleShooting.md", true);
      },

      showFWLog: function () {
         if (this.fw2gui.standalone)
            this.showLog();
         else {
            let idx = this.fw2gui.childs.length -1;
            sap.m.URLHelper.redirect(this.fw2gui.childs[idx].fTitle, true);
         }
      },

      setNavigationStates ()
      {
         // console.log("navigation info ", this.fw2gui.nav);
         if (this.fw2gui.nav.length == 0)
         {
            this.byId("beginEvent").setEnabled(true);
            this.byId("prevEvent").setEnabled(true);
            this.byId("nextEvent").setEnabled(true);
            this.byId("endEvent").setEnabled(true);
         }
         else {
            for (const n of this.fw2gui.nav) {
               console.log(n);
               if (n == "empty") {
                  this.byId("beginEvent").setEnabled(false);
                  this.byId("prevEvent").setEnabled(false);
                  this.byId("nextEvent").setEnabled(false);
                  this.byId("endEvent").setEnabled(false);
                  break;
               }
               else if (n == "last") {

                  this.byId("beginEvent").setEnabled(true);
                  this.byId("prevEvent").setEnabled(true);
                  this.byId("nextEvent").setEnabled(false);
                  this.byId("endEvent").setEnabled(false);
                  break;
               }
               else if (n == "first") {
                  this.byId("beginEvent").setEnabled(false);
                  this.byId("prevEvent").setEnabled(false);
                  this.byId("nextEvent").setEnabled(true);
                  this.byId("endEvent").setEnabled(true);
                  break;
               }
            }
         }

         if (this.fw2gui.autoplay == 1)
         {
            this.byId("beginEvent").setEnabled(false);
            this.byId("prevEvent").setEnabled(false);
            this.byId("nextEvent").setEnabled(false);
            this.byId("endEvent").setEnabled(false);
         }
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

         this.setNavigationStates();
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
         this.mgr.SendMIR("setAutoplay(" + oEvent.getParameter("selected") + ")", this.fw2gui.fElementId, "FW2GUI");
      },

      playdelay: function (oEvent) {
         console.log("playdelay ", oEvent.getParameters());
         let pm = oEvent.getParameter("value") * 1000;
         this.mgr.SendMIR("setPlayDelayInMiliseconds(" + pm + ")", this.fw2gui.fElementId, "FW2GUI");
      },

      addCollectionResponse: function (msg) {
         let sc = this.getView().byId("Summary").getController();
         sc.initAddCollectionEditor(msg, this.fw2gui);
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
            var pthisF = this;
            XMLView.create({
               viewName: "fw.view.EventFilter",
               viewData: { "mgr": this.mgr, "gui": this.fw2gui.childs[0] }
            }).then(function (oView) {
               pthisF.eventFilter = oView.getController();
               pthisF.eventFilter.buildFilterGUI();
               pthisF.eventFilter.openFilterDialog();
            });
         }
      },

      showPreferences: function () {
         if (this.cpref){
            this.cpref.openPrefDialog(this.byId("fwedit"));
         }
         else {
            var pthisCom = this;
            XMLView.create({
               viewName: "fw.view.CommonPreferences",
               viewData: { "mgr": this.mgr, "gui": this.fw2gui.childs[1] }
            }).then(function (oView) {
               pthisCom.cpref = oView.getController();
               //pthis.eventFilter.setGUIElement(pthis.fw2gui);
               pthisCom.cpref.openPrefDialog(pthisCom.byId("fwedit"));

               
            });
         }
      },

      onPressInvMass: function(oEvent)
      {
			var oButton = oEvent.getSource(),
			oView = this.getView();
            let pthis = this;
			// create popover
			if (!this._pPopover) {
				this._pPopover = Fragment.load({
					id: oView.getId(),
					name: "fw.view.InvMassPopover",
					controller: this
            }).then(function (oPopover) {
               oView.addDependent(oPopover);
					return oPopover;
				});
			}
			this._pPopover.then(function(oPopover) {
            pthis.fw2gui.childs[2].w = oPopover;

            let cl = oPopover.getContent();
            cl[0].setHtmlText("<pre>Press \'Calculate\' button to get result \nof current selection state</pre>");
				oPopover.openBy(oButton);
			});
      },
      
      handleInvMassCalcPress : function()
      {
         let inmd =  this.fw2gui.childs[2];
         this.mgr.SendMIR("Calculate()", inmd.fElementId, "FWWebInvMassDialog");
      },

      invMassDialogRefresh : function()
      { 
         let inmd = this.fw2gui.childs[2];
         if (inmd.w) {
         let cl = inmd.w.getContent();
         cl[0].setHtmlText(this.fw2gui.childs[2].fTitle);
         }
      },

   });
});
