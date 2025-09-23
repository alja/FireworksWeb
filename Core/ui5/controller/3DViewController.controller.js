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
    "sap/m/Dialog",
    "sap/m/ProgressIndicator"
], function (Controller, JSONModel, Button, mInput, mStepInput, mCheckBox, Text,
    ColorPalettePopover, HorizontalLayout, FormattedText, ObjectStatus, Dialog,
    ProgressIndicator) {
    "use strict";



    var ViewController = Controller.extend("fw.controller.3DViewController", {
        onInit: function () {

            this.mgr = this.getView().getViewData().mgr;
            this.eveView = this.getView().getViewData().eveView;
            let gui = this.getView().getViewData().fw2gui;


            for (let i = 0; i < gui.childs.length; ++i) {
                let item = gui.childs[i];
                if (item._typename == "FW3DView") {
                    if (this.eveView.fElementId == item.eveViewId) {

                        this.fwView = item;
                        break;
                    }
                }
            }


            //this.eveFilter = this.getView().getViewData().gui;
            var oModel = new sap.ui.model.json.JSONModel();
            oModel.setData(this.fwView);
            
            
            this.getView().setModel(oModel);
        
            this.byId("viewController").open();
            let bbb = this.byId("blackbg");
            bbb.setSelected (this.eveView.BlackBg);
            let bax = this.byId("drawAxis");
            bax.setSelected (Boolean(this.eveView.AxesType));


/*
            let tb = this.byId("iconTab");//.oPopover.getContent()[0];
            // let tb__xmlview0-rrr--header0").getContent()[0];
             tb.removeStyleClass("sapMITH");
             tb.removeStyleClass("sapMITBHead");
             tb.addStyleClass("fwIconTab");*/
        },

        blackBackground: function (oEvent)
        {
            let eli = this.eveView.fElementId;
            let cmd = "SetBlackBackground(" + oEvent.getParameter("selected") + ")";
            this.mgr.SendMIR(cmd, eli, "ROOT::Experimental::REveViewer");
        },

        sendGeoMIR: function (fn, oEvent)
        {
            let eli = this.fwView.fElementId;;
            let cmd = fn + "(" + oEvent.getParameter("selected") + ")";
            this.mgr.SendMIR(cmd, eli, "FW3DView");
        },

        showMuonBarrel: function (oEvent) {
            this.sendGeoMIR("showMuonBarrel", oEvent);
        },
        showMuonEndcap: function (oEvent) {
            this.sendGeoMIR("showMuonEndcap", oEvent);
        },
        showPixelBarrel: function (oEvent) {
            this.sendGeoMIR("showPixelBarrel", oEvent);
        },
        showPixelEndcap: function (oEvent) {
            this.sendGeoMIR("showPixelEndcap", oEvent);
        },
        showTrackerBarrel: function (oEvent) {
            this.sendGeoMIR("showTrackerBarrel", oEvent);
        },
        showTrackerEndcap: function (oEvent) {
            this.sendGeoMIR("showTrackerEndcap", oEvent);
        },
        showEcalBarrel: function (oEvent) {
            this.sendGeoMIR("showEcalBarrel", oEvent);
        },
        showEventLabel: function (oEvent) {
            this.sendGeoMIR("showEventLabel", oEvent);
        },
        setLineWidth: function (oEvent)
        {
            console.log("set line widtg ", oEvent);
        },
        drawAxis: function (oEvent)
        { 
            let eli = this.eveView.fElementId;
            let cmd = "SetAxesType(" + oEvent.getParameter("selected") + ")";
            this.mgr.SendMIR(cmd, eli, "ROOT::Experimental::REveViewer");
        },
        onClose: function(oEvent) {
            this.byId("viewController").close();
        },
        onSetCamera: function(oEvent)
        {
            try {
                let arr = JSON.parse(this.byId("mtxIn").getValue());
                let cc = this.eveView.ca.oController.viewer.controls;
                cc.setCamTrans(arr);
                cc.update();
                this.eveView.ca.oController.viewer.request_render();

            }
            catch (err) {
                console.error("faled to parse array");
                this._showErrorDialog("JSON Parse Error", err.message);
            }

        },
        onGetCamera: function(oEvent)
        {
            let ca = this.eveView.ca;
            let cc = ca.oController.viewer.controls;
            let arr = ca.oController.viewer.controls.getCamTrans().elements;
            let msg = JSON.stringify(arr);
            this.byId("mtxDump").setValue(msg);
        },
        _showErrorDialog: function (sTitle, sMessage) {
            const oDialog = new Dialog({
                title: sTitle,
                type: "Message",
                content: new Text({ text: sMessage }),
                beginButton: new Button({
                    text: "Close",
                    press: function () {
                        oDialog.close();
                    }
                }),
                afterClose: function () {
                    oDialog.destroy();
                }
            });

            oDialog.open();
        }
    });


    return ViewController;

});
