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
            /*
            this.eveFilter = this.getView().getViewData().gui;
            var oModel = new sap.ui.model.json.JSONModel();
            var pthis = this;
            oModel.setData({ modelData: this.eveFilter.collection, hltData: this.eveFilter.HLT, filterMode: this.eveFilter.filterMode });
            
                 oModel.attachPropertyChange( function(oEvent){
                     console.log("event: ", oEvent);
                     pthis.applyButton.setType(sap.m.ButtonType.Emphasized);
                  }, this); 
            
            this.byId("view").setModel(oModel);
            */
           this.mgr = this.getView().getViewData().mgr;
           this.eveView = this.getView().getViewData().eveView;
            this.byId("viewController").open();
        }

    });

    return ViewController;

});
