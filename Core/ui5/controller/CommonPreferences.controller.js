sap.ui.define([
    "sap/ui/core/mvc/Controller",
    "sap/ui/model/json/JSONModel",
    "sap/m/Button",
    "sap/m/Input",
    "sap/m/StepInput",
    "sap/m/CheckBox",
    "sap/m/Text",
    "sap/m/ColorPalettePopover",
    "sap/ui/layout/HorizontalLayout"
], function (Controller, JSONModel, Button, mInput, mStepInput, mCheckBox, mText, ColorPalettePopover, HorizontalLayout) {
    "use strict";



    var CommonPreferenceController = Controller.extend("fw.controller.CommonPreferences", {

        onInit: function () {
            this.eveScale = this.getView().getViewData().gui.childs[0];
            let model = new sap.ui.model.json.JSONModel(
                { plotEt: true, mode: this.eveScale.mode, maxH: this.eveScale.maxH, valToH: this.eveScale.valToH }
            );

            this.getView().setModel(model, "EModel");

            let cc = this;
            model.attachPropertyChange(function (oEvent) {
                console.log("Scale changed event ", oEvent);
                let fd = cc.getView().getModel("EModel").getData();
                let cont = JSON.stringify(fd);
                let xxx = btoa(cont);
                let cmd = "ScaleChanged(\"" + xxx + "\")";
                cc.getView().getViewData().mgr.SendMIR(cmd, cc.eveScale.fElementId, "FWViewEnergyScale");
            }, this);

            // at the moment view setting and energyscales are in the same class
            let drawBarrel = this.eveScale.drawBarrel;
            this.byId("drawBarrel").setSelected(this.eveScale.drawBarrel);
        },

        setFilterStatusFromEveElement: function () {
        },

        openPrefDialog: function (wId) {
            //this.byId("prefDialog").open();
            this.byId("prefDialog").openBy(wId);
        },

        setBackground: function (oEvent) {
            let cmd = "setBackground(" + oEvent.getParameter("selected") + ")";
            let d = this.getView().getViewData();
            d.mgr.SendMIR(cmd, d.gui.fElementId, "FW2CommonPref");
        }

    });


    return CommonPreferenceController;

});
