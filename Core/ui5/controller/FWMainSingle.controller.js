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

   return MainController.extend("rootui5.eve7.controller.MainMini", {
      onInit: function () {
         this.mgr = new EveManager();
         var myv = this.getView();
         let conn_handle = Component.getOwnerComponentFor(myv).getComponentData().conn_handle;
         this.mgr.UseConnection(conn_handle);

         this.mgr.RegisterController(this);

         console.log(this, this.getView(), this.getView().byId("DaMain"));

         this.primarySplitter = this.getView().byId("DaMain");
      },

      updateViewers: function(loading_done) {
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
            if(eveView.subscribed)
               this.makeEveViewController(eveView);
            else
              this.subscribeView(eveView);
         }


      },


      makeEveViewController: function(elem)
      {
         let myv = this.getView();

         const urlParams = new URLSearchParams(window.location.search);
         let sgv_undock = urlParams.get('Single');
         if (elem.fName != sgv_undock)
            return;


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

         this.primarySplitter.addContentArea(view);
         elem.ca = view;

         // reset flag needed by UT_PostStream callback
         delete elem.pendInstance;
      },


      subscribeView: function(viewer)
      {
         viewer.subscribed = true;
         viewer.pendInstance = true;

         this.mgr.SendMIR("ConnectClient()", viewer.fElementId, "ROOT::Experimental::REveViewer");
      },

      onEveManagerInit: function() {
         this.updateViewers();
      },

   });
});
