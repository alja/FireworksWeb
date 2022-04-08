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

      updateViewers: function(loading_done) {
         var viewers = this.mgr.FindViewers();

         // first check number of views to create
         var staged = [];
         for (var n=1;n<viewers.length;++n) {
            var el = viewers[n];
            //if (!el.$view_created && el.fRnrSelf) staged.push(el);
            if (!el.$view_created ) staged.push(el);
         }

         // console.log("FOUND viewers", viewers.length, "not yet exists", staged.length);

         var vMenu = this.getView().byId("menuViewId");
         vMenu.removeAllItems();
         {
            for (var n = 0; n < staged.length; ++n) {
               let ipath = staged[n].fRnrSelf ? "sap-icon://decline" : "sap-icon://accept";
               let vi = new mMenuItem({ text: staged[n].fName });
               vMenu.addItem(vi);
               vi.addItem(new mMenuItem({ text: "Switch Visible", icon: ipath, press: this.switchViewVisibility.bind(this, staged[n]) }));
               vi.addItem(new mMenuItem({ text: "Switch Sides", icon: "sap-icon://resize-horizontal",   press: this.switchViewSides.bind(this, staged[n])}));
               vi.addItem(new mMenuItem({ text: "Single", icon: "sap-icon://expand",  press: this.switchSingle.bind(this, staged[n]) }));
            }
         }

         var main = this, vv = null, sv = this.getView().byId("MainAreaSplitter");

         for (var n=0;n<staged.length;++n) {
            var elem = staged[n];

            var viewid = "EveViewer" + elem.fElementId;

            // create missing view
            elem.$view_created = true;
            console.log("Creating view", viewid);

            var oLd = undefined;
            if ((n == 0) && (staged.length > 1))
               oLd = new SplitterLayoutData({ resizable: true, size: "50%" });

            var vtype = "rootui5.eve7.view.GL";
            if (elem.fName === "Table")
               vtype = "rootui5.eve7.view.EveTable"; // AMT temporary solution
            else if (elem.fName === "Lego")
               vtype = "rootui5.eve7.view.Lego"; // AMT temporary solution

            var oOwnerComponent = Component.getOwnerComponentFor(this.getView());
            var view = oOwnerComponent.runAsOwner(function() {
               return new sap.ui.xmlview({
                  id: viewid,
                  viewName: vtype,
                  viewData: { mgr: main.mgr, eveViewerId: elem.fElementId },
                  layoutData: oLd
               });
            });

            if (elem.fRnrSelf) {
               if (sv.getContentAreas().length == 1) {
                  sv.addContentArea(view);
                  continue;
               }
               else {
                  if (!vv) {
                     vv = new Splitter("SecondaryViewSplitter", { orientation: "Vertical" });
                     sv.addContentArea(vv);
                  }

                  vv.addContentArea(view);
               }
            }
            elem.ca = view
         }
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
         sap.m.URLHelper.redirect("https://github.com/alja/FireworksWeb/blob/july/doc/UserGuide.md#table-of-contents", true);
      },
      troubleshoot: function (oEvent) {
         sap.m.URLHelper.redirect("https://github.com/alja/FireworksWeb/blob/july/doc/TroubleShooting.md", true);
      },

      showFWLog: function () {
         if (this.fw2gui.standalone)
            this.showLog();
         else {
            let idx = this.fw2gui.childs.length -1;
            sap.m.URLHelper.redirect(this.fw2gui.childs[idx].fTitle, true);
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

		   if (this.acGUI) {
			   this.acGUI.open();
		   }
		   else {
			   let pthis = this;
			   XMLView.create({
				   viewName: "fw.view.AddCollection",
				   viewData: { d: msg.arr, m : this.mgr, gId : this.fw2gui.fElementId }
			   }).then(function (oView) {
				   pthis.acGUI = oView.getController().dialog;
				   pthis.acGUI.open();
			   });
		   }

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
         let cl = inmd.w.getContent();
         cl[0].setHtmlText(this.fw2gui.childs[2].fTitle);
      },

   });
});
