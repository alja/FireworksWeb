sap.ui.define(['rootui5/eve7/controller/Ged.controller',
"sap/ui/core/mvc/XMLView"
], function(GedController, XMLView) {
   "use strict";

   return GedController.extend("fw.FireworksGed", {

      onInit: function() {
         GedController.prototype.onInit.apply(this, arguments);

         let oModel = new sap.ui.model.json.JSONModel();
         this.getView().setModel(oModel, "edm")
        },

     // associations
    buildFWEveAssociationSetter: function (el) {
         this.makeBoolSetter(el.fRnrSelf, "Active");
         let ged = this.getView().byId("GED");
         this.oModel.setProperty("/title", this.editorElement.fName + "(" + this.editorElement._typename + ")");

         let label = new sap.m.Text({ text: "Qualitiy type " + this.editorElement.qtype });
         label.addStyleClass("sapUiTinyMargin");
         ged.addContent(label);
         this.makeStringSetter(el.FilterExpr, "Filter", "SetFilterExpr");
         let si = ged.getContent()[2].getContent()[0];
         if (this.editorElement.qtype == "float")
         si.setTooltip("Insert Value e.g i > 0.5");
         else
         si.setTooltip("Insert Value e.g i.first > 0.5");
     },

     // event item
     buildFWWebEventItemSetter : function (el) {
         this.buildREveDataCollectionSetter(el);
         this.oModel.setProperty("/title", this.editorElement.fName + " Editor");
         if (el.var) {
         el.var.forEach((par) => {
             let fname = "makePBC" + par.type + "Setter";
             this[fname](par);
         });
         }
         console.log("About to add layer management controls");
         this.addLayerManagementControls();
         console.log("Layer management controls added");
     },

     addLayerManagementControls: function() {
        let gedFrame = this.getView().byId("GED");
        let gcm = this;
        
        // Add a separator
        let separator = new sap.m.ToolbarSpacer({ height: "10px" });
        gedFrame.addContent(separator);
        
        // Add "Layer Management" label
        let titleLabel = new sap.m.Text({ 
           text: "Layer Management",
           class: "sapUiSmallMarginTop"
        });
        titleLabel.addStyleClass("sapUiTinyMargin");
        gedFrame.addContent(titleLabel);
        
        // Add "Bring to Front" button
        let bringToFrontBtn = new sap.m.Button({
           text: "Bring to Front",
           type: "Emphasized",
           width: "150px",
           press: function() {
              gcm.onBringToFront();
           }
        });
        
        let bringToFrontFrame = new sap.ui.layout.HorizontalLayout({
           content: [bringToFrontBtn]
        });
        gedFrame.addContent(bringToFrontFrame);
        
        // Add "Set Layer" input and button
        let layerInput = new sap.m.Input({
           id: this.createId("layerInput"),
           type: "Number",
           width: "100px",
           value : gcm.editorElement.fLayer,
           placeholder: "Enter layer"
        });
        
        let setLayerBtn = new sap.m.Button({
           text: "Apply",
           type: "Emphasized",
           press: function() {
              let value = layerInput.getValue();
              if (value && !isNaN(value)) {
                 let intValue = parseInt(value);
                 console.log("Setting layer to:", intValue, "for element:", gcm.editorElement.fElementId);
                 
                 //gcm.mgr.SendMIR("SetLayer(" + intValue + ")", gcm.editorElement.fElementId, "FWWebEventItem");
                 gcm.mgr.SendMIR("UpdateLayer(" + intValue + ")", gcm.editorElement.fElementId, "FWWebEventItem");
                 sap.m.MessageToast.show("Layer set to " + intValue);
                 
                 // Clear the input
                 layerInput.setValue("");
              } else {
                 sap.m.MessageBox.error("Please enter a valid layer number");
              }
           }
        });
        
        let setLayerLabel = new sap.m.Label({ 
           text: "Set Layer:",
           labelFor: layerInput
        });
        setLayerLabel.addStyleClass("sapUiTinyMargin");
        
        let setLayerFrame = new sap.ui.layout.HorizontalLayout({
           content: [setLayerLabel, layerInput, setLayerBtn]
        });
        gedFrame.addContent(setLayerFrame);
     },
     
     // NEW: Handle bring to front button
     onBringToFront: function() {
        if (this.editorElement && this.editorElement.fElementId) {
           console.log("Bringing to front:", this.editorElement.fElementId);
           this.mgr.SendMIR("BringToFront()", this.editorElement.fElementId, "FWWebEventItem");
           sap.m.MessageToast.show("Collection brought to front");
        } else {
           sap.m.MessageBox.error("No collection selected");
        }
     },

    // bool setter
   makePBCBoolSetter :  function(par)
    {
        let gedFrame = this.getView().byId("GED");
        let gcm = this;
        let widget = new sap.m.CheckBox({
        selected: par.val,
            
        select: function (oEvent) {
            console.log("Bool setter select event", oEvent.getSource());
            let funcName = "UpdatePBParameter";
            let value = oEvent.getSource().getSelected();
            let mir = funcName + "( \"" + oEvent.getSource().desc + "\",\"" + value + "\" )";
            gcm.mgr.SendMIR(mir, gcm.editorElement.fElementId, gcm.editorElement._typename);
        }
        });

        widget.desc = par.name;
            
        let label = new sap.m.Text({ text: par.name });
        label.addStyleClass("sapUiTinyMargin");
            
        let frame = new sap.ui.layout.HorizontalLayout({
        content: [widget, label]
        });
            
        gedFrame.addContent(frame);
    },

    makePBCLongSetter : function(par)
    {
        let gedFrame = this.getView().byId("GED");
        let gcm = this;
        let widget = new sap.m.StepInput({
        min: par.min,
        max: par.max,
        value: par.val,
        change: function (oEvent) {
            console.log("Long setter select event", oEvent.getSource());
            let funcName = "UpdatePBParameter";
            let mir = funcName + "( \"" + oEvent.getSource().desc + "\",\"" + oEvent.getParameter("value") + "\" )";
            gcm.mgr.SendMIR(mir, gcm.editorElement.fElementId, gcm.editorElement._typename);
        }
        });

     widget.desc = par.name;
        
     let label = new sap.m.Text({ text: par.name });
     label.addStyleClass("sapUiTinyMargin");
        
     let frame = new sap.ui.layout.HorizontalLayout({
     content: [widget, label]
     });
        
     gedFrame.addContent(frame);
    },
 
    makePBCDoubleSetter : function(par)
    {  
     let gcm = this;
     let widget = new sap.m.Input({
     value: par.val,
     tooltip: "limit value [ " + par.min + ", " + par.max + "]",
     change: function (oEvent) {
         console.log("Long setter select event", oEvent.getSource());
         
         // validation
         let ival = oEvent.getParameter("value");
         
         if (ival > par.max || ival < par.min) {
             let iControl = oEvent.getSource();
             iControl.setValueState(sap.ui.core.ValueState.Error);
         }
         else {
             let funcName = "UpdatePBParameter";
             let mir = funcName + "( \"" + oEvent.getSource().desc + "\",\"" + oEvent.getParameter("value") + "\" )";
             gcm.mgr.SendMIR(mir, gcm.editorElement.fElementId, gcm.editorElement._typename);
         }
     }
     });

     widget.desc = par.name;
        
     let label = new sap.m.Text({ text: par.name });
     label.addStyleClass("sapUiTinyMargin");
        
     let frame = new sap.ui.layout.HorizontalLayout({
     content: [widget, label]
     });
        
     let gedFrame = this.getView().byId("GED");
     gedFrame.addContent(frame);
    },

    showFWGedEditor : function(sumSplitter, elementId, showInfo)
    {
       this.showGedEditor(sumSplitter, elementId);
       let info = this.byId("edmInfo");
       info.setVisible(showInfo);
    },

    showEDMInfo : function(oEvent)
    {  
	    let oButton = oEvent.getSource();
        let oView = this.getView();
        let pthis = this;
        // create popover
        if (!this._pPopover) {
            this._pPopover = sap.ui.core.Fragment.load({
                id: oView.getId(),
                name: "fw.view.CollectionInfo",
                controller: this
        }).then(function (oPopover) {
           oView.addDependent(oPopover);
                return oPopover;
            });
        }
        this._pPopover.then(function(oPopover) {
           let m = pthis.getView().getModel("edm");
           let d = m.getData();
           d.edm = pthis.editorElement.edmInfo;
           m.updateBindings();
           oPopover.openBy(oButton);
        });
    }


    });
}); 