sap.ui.define(['rootui5/eve7/controller/Main.controller','sap/ui/core/Component',
               'sap/ui/core/UIComponent',
               'sap/ui/core/mvc/Controller',
               'sap/ui/layout/Splitter',
               'sap/ui/layout/SplitterLayoutData',
               'sap/m/library',
               'sap/m/Button',
               'sap/m/MenuItem',
               'sap/m/MessageBox',
               'rootui5/eve7/lib/EveManager',
               "sap/ui/core/mvc/XMLView",
               'sap/ui/model/json/JSONModel'
], function(MainController, Component, UIComponent, Controller, Splitter, SplitterLayoutData, MobileLibrary, mButton, mMenuItem, MessageBox, EveManager, XMLView, JSONModel) {

   "use strict";

   return MainController.extend("fw.controller.MainSingle", {
      onInit: function () {
         this.mgr = new EveManager();
         var myv = this.getView();
         let conn_handle = Component.getOwnerComponentFor(myv).getComponentData().conn_handle;
         this.mgr.UseConnection(conn_handle);

         this.mgr.RegisterController(this);

         console.log(this, this.getView(), this.getView().byId("DaMain"));

         this.primarySplitter = this.getView().byId("DaMain");

         var bc = new BroadcastChannel('test_channel');

         bc.onmessage = function (ev) { console.log(ev); window.close();} /* receive */
         this.BCH = bc;
      },

      updateViewers: function(loading_done) {

         console.log("single ... update vuewers \n");
         let viewers = this.mgr.FindViewers();

         // first check number of views to create
         let staged = [];
         for (let n=0;n<viewers.length;++n) {
            let el = viewers[n];
            // at startup show only mandatory views
            if (typeof el.subscribed == 'undefined')
               el.subscribed = el.Mandatory;

            if (!el.$view_created && el.fRnrSelf) staged.push(el);
         }
         if (staged.length == 0) return;

         let vMenu = this.getView().byId("menuViewId");

         for (let n=0;n<staged.length;++n) {
            let eveView = staged[n];

            eveView.$view_created = true;

           // if(eveView.subscribed)
               this.makeEveViewController(eveView);
          //  else
            //  this.subscribeView(eveView);
         }
      },


      makeEveViewController: function(elem)
      {
         let myv = this.getView();

         const urlParams = new URLSearchParams(window.location.search);
         let sgv_undock = urlParams.get('Single');
         if (elem.fName != sgv_undock)
            return;

         this.singleEveViewer = elem;

         let viewid = "EveViewer" + elem.fElementId;
         let main = this;

         let vtype = "rootui5.eve7.view.GL";
         let reservedNames = ["Table", "EveTable", "TriggerTable", "GeoTable", "Lego"];


         for (const rn of reservedNames) {
            if (rn === elem.fName) {

               if (rn === "Table")
                  vtype = "rootui5.eve7.view.EveTable";
               else
                   vtype = "fw.view." + rn;
              break; // Stop iterating
            }
          }

         let oOwnerComponent = Component.getOwnerComponentFor(this.getView());
         let view = oOwnerComponent.runAsOwner(function() {
            return new sap.ui.xmlview({
               id: viewid,
               viewName: vtype,
               viewData: { mgr: main.mgr, eveViewerId: elem.fElementId },
            });
         });

         // hide view's toolbar
         let tt = view.byId("tbar");
         tt.setVisible(false);

         let bar = this.byId("titleLabel");
         bar.setText(elem.fName);

         this.primarySplitter.addContentArea(view);
         elem.ca = view;

         // reset flag needed by UT_PostStream callback
         delete elem.pendInstance;
         this.addInfoController(view, elem);
      },


      subscribeView: function(viewer)
      {
         viewer.subscribed = true;
         viewer.pendInstance = true;
      },

      onEveManagerInit: function() {
         this.updateViewers();
         this.mgr.SendMIR("ConnectClient()", viewer.fElementId, "ROOT::Experimental::REveViewer");
      },

      onEveManagerInit: function() {
         this.updateViewers();
      },

      addInfoController: function (ui5view, eveView)
      {
         let bar = this.byId("mbar");
         let pthisF = this;
         var world = this.mgr.childs[0].childs;
         var last = world.length - 1;
         // console.log("init gui ", last, world);
         var last = world.length - 1;
         if (world[last]._typename == "FW2GUI") {
            this.fw2gui = (world[last]);
         }
         var bb = new sap.m.Button({
            type: sap.m.ButtonType.Default,
            icon:"sap-icon://message-information",
            tooltip: "view controller",
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
                  //viewData: { "mgr": pthisF.mgr, "eveView" : eveView}
               }).then(function (oView) {
               });

            }
         });
         bar.insertContent(bb, 1);
      },


      dock: function () {
         var bc = new BroadcastChannel('test_channel');
         bc.postMessage(this.singleEveViewer.fElementId);
      }

   });
});