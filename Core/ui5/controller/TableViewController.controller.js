sap.ui.define([
    "sap/ui/core/mvc/Controller",
    "sap/ui/model/json/JSONModel",
    "sap/m/Button",
    "sap/m/Input",
    "sap/m/StepInput",
    "sap/m/CheckBox",
    "sap/m/Text",
    "sap/m/ColorPalettePopover",
    "sap/ui/layout/HorizontalLayout",
    "sap/m/FormattedText",
    "sap/m/ObjectStatus",
    "sap/m/ProgressIndicator"
], function (Controller, JSONModel, Button, mInput, mStepInput, mCheckBox, mText,
    ColorPalettePopover, HorizontalLayout, FormattedText, ObjectStatus,
    ProgressIndicator) {
    "use strict";



    var ViewController = Controller.extend("fw.controller.ViewController", {
        onInit: function () {
            this.mgr = this.getView().getViewData().mgr;
            this.eveView = this.getView().getViewData().eveView;


            if (this.eveView.fName === "TriggerTable")
            {
                this.getView().byId("info").setVisible(false);
            }

            this.byId("viewController").open();
        },

        locateEveTable: function () {
         this.eveTable = 0;
         let element = this.eveView;//this.mgr.GetElement(this.eveViewerId);
         let sceneInfo = element.childs[0];
         let scene = this.mgr.GetElement(sceneInfo.fSceneId);
         // console.log(">>>table scene", scene);
         if (scene.childs[0]._typename == "ROOT::Experimental::REveTableViewInfo") {
            // presume table view manger is first child of table scene
            this.viewInfo = scene.childs[0];
         }

         this.collection = this.mgr.GetElement(this.viewInfo.fDisplayedCollection);
         // loop over products
         for (let i = 1; i < scene.childs.length; ++i) {
            let product = scene.childs[i];
            if (product.childs && product.childs.length && product.childs[0].fCollectionId == this.viewInfo.fDisplayedCollection) {
               // console.log("table found  ",product.childs[0] );
               this.eveTable = product.childs[0];
               break;
            }
         }
      },

      print: function() {
        this.locateEveTable();
        this.mgr.SendMIR("PrintTable()", this.eveTable.fElementId, "ROOT::Experimental::REveDataTable");
      },
      onClose: function(oEvent) {
          this.byId("viewController").close();
      }
    });

    return ViewController;

});
