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



    var ViewController = Controller.extend("fw.controller.RPZViewController", {
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
            
            this.getView().setModel(oModel);
            this.byId("viewController").open();

            // depth
            this.currentDepth = -100;
            this.depthIncrement = -10;
        /*
            let bbb = this.byId("blackbg");
            bbb.setSelected (this.eveView.BlackBg);
        */
        },

        blackBackground: function (oEvent)
        {
            let eli = this.eveView.fElementId;
            let cmd = "SetBlackBackground(" + oEvent.getParameter("selected") + ")";
            this.mgr.SendMIR(cmd, eli, "ROOT::Experimental::REveViewer");
            this.mgr.SendMIR("bgChanged(" + oEvent.getParameter("selected") + ")", this.fwView.fElementId, "FW3DView");
        },

        sendGeoMIR: function (fn, oEvent)
        {
            let eli = this.fwView.fElementId;
            //let isSelected = oEvent.getParameter("selected");
            let cmd = fn + "(" + oEvent.getParameter("selected") + ")";
            this.mgr.SendMIR(cmd, eli, "FWRPZView");

            /*
            // depth
            if (isSelected) {
                this.bringToFront(eli, fn);
            }
            */
        },

        bringToFront: function(elementId, geometryType) {
            // Assign progressively more forward depth
            this.currentDepth += this.depthIncrement;
            
            // Send depth command to the specific geometry element
            let depthCmd = "setCurrentDepth(" + this.currentDepth + ")";
            this.mgr.SendMIR(depthCmd, elementId, "FWRPZView");
            
            console.log("Setting depth", this.currentDepth, "for", geometryType);
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
        showProjectionAxes: function (oEvent) {
            this.sendGeoMIR("showProjectionAxes", oEvent);
        },
        onProjectionAxesFontSizeChange: function (oEvent) {
            let cmd = "setProjectionAxesFontSize(" + oEvent.getParameter("value") + ")";
            this.mgr.SendMIR(cmd, this.fwView.fElementId, "FWRPZView");
        },
        showEventLabel: function (oEvent) {
            let eli = this.fwView.fElementId;
            let cmd = "showEventLabel(" + oEvent.getParameter("selected") + ")";
            this.mgr.SendMIR(cmd, eli, "FW3DView");
        },
        setEtaRange: function (oEvent) {
            let eli = this.fwView.fElementId;
            let cmd = "setEtaRng(" + oEvent.getParameter("selected") + ")";
            this.mgr.SendMIR(cmd, eli, "FWRPZView");
        },
        setLineWidth: function (oEvent)
        {
            console.log("set line widtg ", oEvent);
        },

        // fish eye distortion
        // fish eye steppers: << < value > >>, step and limits come from the button's custom data
        onFishEyeStep: function(oEvent) {
            let btn = oEvent.getSource();
            let prop = "/" + btn.data("prop");
            let model = this.getView().getModel();
            let value = Number(model.getProperty(prop)) + Number(btn.data("step"));
            value = Math.min(Math.max(value, 0), Number(btn.data("max")));
            // nothing to send at the range limits, and no update would come back to reset the camera
            if (value == model.getProperty(prop))
                return;
            model.setProperty(prop, value);

            let cmd = btn.data("fn") + "(" + value + ")";
            this.mgr.SendMIR(cmd, this.fwView.fElementId, "FWRPZView");
            this.resetCameraAfterUpdate();
        },

        // The reprojected scene arrives asynchronously, so reset the camera of this
        // view's GL viewer once the server's next EndChanges has been applied.
        resetCameraAfterUpdate: function() {
            let mgr = this.mgr;
            let glctrl = (mgr.gl_controllers || []).find(c => c.eveViewerId == this.eveView.fElementId);
            if (!glctrl)
                return;

            if (!mgr.$fwPendingCameraResets) {
                mgr.$fwPendingCameraResets = new Set();
                let origCallback = mgr.ServerEndRedrawCallback;
                mgr.ServerEndRedrawCallback = function() {
                    origCallback.apply(this, arguments);
                    for (let c of this.$fwPendingCameraResets) {
                        let v = c.viewer;
                        // scene_bbox exists only after the viewer has finished bootstrapping
                        if (v && v.scene_bbox && typeof v.resetCamera === "function") {
                            v.resetCamera();
                            v.request_render();
                        }
                    }
                    this.$fwPendingCameraResets.clear();
                };
            }
            mgr.$fwPendingCameraResets.add(glctrl);
        },

        onClose: function(oEvent) {
            this.byId("viewController").close();
        }
    });


    return ViewController;

});
