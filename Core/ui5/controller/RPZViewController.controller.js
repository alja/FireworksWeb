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
            let gui = this.getView().getViewData().fw2gui;


            for (let i = 0; i < gui.childs.length; ++i) {
                let item = gui.childs[i];
                if (item._typename == "FWRPZView") {
                    if (this.eveView.fElementId == item.eveViewId) {

                        this.fwView = item;
                        break;
                    }
                }
            }


            //this.eveFilter = this.getView().getViewData().gui;
            var oModel = new sap.ui.model.json.JSONModel();
            oModel.setData(this.fwView);
            
            this.byId("vname").setText( this.eveView.fName + " Contoller");

            this.byId("viewController").open();
            let bbb = this.byId("blackbg");
            bbb.setSelected (this.eveView.BlackBg);
            let bax = this.byId("drawAxis");
            bax.setSelected (Boolean(this.eveView.AxesType));
            
            this.getView().setModel(oModel);
            this.byId("viewController").open();
        /*
            let bbb = this.byId("blackbg");
            bbb.setSelected (this.eveView.BlackBg);
        */
/*
            if (this.eveView.fName === "RPhi")
            {
                this.getView().byId("rhoz").setVisible(false);
            }
*/
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
            this.mgr.SendMIR(cmd, eli, "FWRPZView");
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
        showRpcEndcap: function (oEvent) {
            this.sendGeoMIR("showRpcEndcap", oEvent);
        },
        showGEM: function (oEvent) {
            this.sendGeoMIR("showGEM", oEvent);
        },
        showME0: function (oEvent) {
            this.sendGeoMIR("showME0", oEvent);
        },
        showMtdEndcap: function (oEvent) {
            this.sendGeoMIR("showMtdEndcap", oEvent);
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
        }
    });


    return ViewController;

});
