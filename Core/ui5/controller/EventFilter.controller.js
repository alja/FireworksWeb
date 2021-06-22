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
 


   var EventFilterController = Controller.extend("fw.controller.EventFilter", {

      onInit: function() {
          console.log("filter controller");
          console.log('EveFilter view Daya',this.getView().getViewData()); 

          // let vdata = this.getView().getViewData();
          this.eveFilter = this.getView().getViewData().gui;
          var oModel = new sap.ui.model.json.JSONModel();
          oModel.setData({ modelData: this.eveFilter.collection, hltData: this.eveFilter.HLT });
          this.byId("filterDialog").setModel(oModel);
      },

       openFilterDialog: function () {
           console.log("open filter dialog");
           this.byId("filterDialog").open();
       },

       makeTables: function () {
           this.makePlainTable();
           this.makeHLTTable();
           let dialog = this.byId("filterDialog");

           // Simple RadioButtonGroup
           var oRBGroupRBG1 = new sap.m.RadioButtonGroup("RBG1");
           oRBGroupRBG1.setTooltip("Group 1");
           oRBGroupRBG1.setColumns(2);
           oRBGroupRBG1.attachSelect(this.handleModeSelect);

           var oButton = new sap.m.RadioButton("RB1-1");
           oButton.setText("AND");
           oButton.setTooltip("Tooltip 1");
           oRBGroupRBG1.addButton(oButton);

           oButton = new sap.m.RadioButton("RB1-2");
           oButton.setText("OR");
           oButton.setTooltip("Tooltip 2");
           oRBGroupRBG1.addButton(oButton);
           oRBGroupRBG1.setEnabled(false);

           let pthis = this;
           let bar = new sap.m.Bar({
               contentLeft: [
                   new sap.m.Label({
                       text: 'Enabled:',
                       enabled: false
                   }),
                   new sap.m.CheckBox({ selected: false, select: function (e) { pthis.setFilterEnabled(e); } }),
                   new sap.m.Label({
                       text: 'Mode:'
                   }),
                   oRBGroupRBG1
               ]
           });

           let beginButton = new sap.m.Button('simpleDialogAcceptButton', { text: "PublishFilters", press: function () { pthis.publishFilters(); } });
           let endButton = new sap.m.Button('simpleDialogCancelButton', { text: "Close", press: function () { pthis.closeFilterDialog(); } });
           dialog.setEndButton(endButton);
           dialog.setBeginButton(beginButton);
       },
       makePlainTable: function () {
           var aColumns = [
               new sap.m.Column({
                   header: new sap.m.Label({
                       text: "Expression"
                   })
               }),
               new sap.m.Column({
                   hAlign: "Center",
                   header: new sap.m.Label({
                       text: "Active"
                   })
               }),
               new sap.m.Column({
                   header: new sap.m.Label({
                       text: "Pass"
                   })
               })
           ];

           var oTemplate = new sap.m.ColumnListItem({
               vAlign: "Middle",
               type: "{type}",
               cells: [
                   new sap.m.Input({
                       value: "{expr}",
                       wrapping: false
                   }),
                   new sap.m.CheckBox({
                       selected: "{enabled}"
                   }),
                   new sap.m.Label({
                       text: "{selected}"
                   })
               ]
           });

           let pthis = this;
           var oTable = new sap.m.Table({
               growing: true,
               growingThreshold: 7,
               mode: sap.m.ListMode.Delete,
               growingScrollToLoad: true,
               columns: aColumns,
               "delete": function (oEvent) {
                   var oItem = oEvent.getParameter("listItem");
                   let path = oItem.getBindingContext().sPath;
                   let model = pthis.byId("filterDialog").getModel();
                   let suf = "/modelData/";
                   let idx = path.substring(suf.length);
                   model.oData["modelData"].splice(idx,1);
                   model.refresh();
               }
           });

           oTable.bindItems({
               path: "/modelData",
               template: oTemplate,
               key: "id"
           });


           var oAddButton = new sap.m.Button({
               icon: "sap-icon://sys-add",
               press: function (oEvent) {
                   let nv = { id: Math.random(), name: "", checked: true, rating: 0, type: "Inactive" }
                   
                   var aData = this.getModel().getProperty("/modelData");
                   aData.push(nv);
                   this.getModel().setProperty("/modelData", aData);
               }
           });

           this.byId("plainPanel").addContent(oTable);
           this.byId("plainPanel").addContent(oAddButton);
        },

        makeHLTTable: function()
        {
           var aHLTColumns = [
               new sap.m.Column({
                   width: "160px",
                   header: new sap.m.Label({
                       text: "Process"
                   })
               }),
               new sap.m.Column({
                   header: new sap.m.Label({
                       text: "Expression"
                   })
               }),
               new sap.m.Column({
                   header: new sap.m.Label({
                       text: "Active"
                   })
               }),
               new sap.m.Column({
                   width: "100px",
                   header: new sap.m.Label({
                       text: "Pass"
                   })
               }),
           ];


           var oHLTTemplate = new sap.m.ColumnListItem({
               type: "{type}",
               detailPress: function () {
                   setTimeout(function () {
                       sap.m.MessageToast.show("detail is pressed");
                   }, 10);
               },
               cells: [
                   new sap.m.ComboBox({
                       width: "150px",
                       items: [
                           {
                               "key": "RECO",
                               "text": "RECO"
                           },
                           {
                               "key": "SIM",
                               "text": "SIM"
                           },
                           {
                               "key": "HLT",
                               "text": "HLT"
                           }],
                       selectedKey: "{trigger}"
                   }),
                   new sap.m.Input({
                       value: "{expr}",
                       wrapping: false
                   }),
                   new sap.m.CheckBox({
                       selected: "{enabled}"
                   }),
                   new sap.m.Label({
                       text: "{selected}"
                   })
               ]
           });

           let oHLTTable = new sap.m.Table({
               growing: true,
               growingScrollToLoad: true,
               mode: sap.m.ListMode.Delete,
               columns: aHLTColumns,
               "delete": function (oEvent) {
                   var oItem = oEvent.getParameter("listItem");
                   let path = oItem.getBindingContext().sPath;
                   let model = pthis.byId("filterDialog").getModel();
                   let suf = "/hltData/";
                   let idx = path.substring(suf.length);
                   model.oData["hltData"].splice(idx,1);
                   model.refresh();
               }
           });

           oHLTTable.bindItems({
               path: "/hltData",
               mode: sap.m.ListMode.None,
               template: oHLTTemplate,
               key: "id"
           });


           var oHLTAddButton = new sap.m.Button({
               icon: "sap-icon://sys-add",
               press: function (oEvent) {
                   let nv = { id: Math.random(), name: "", checked: true, rating: 0, type: "Inactive" };
                   var aData = this.getModel().getProperty("/hltData");
                   aData.push(nv);
                   this.getModel().setProperty("/hltData", aData);
               }
           });

           this.byId("htmlPanel").addContent(oHLTTable);
           this.byId("htmlPanel").addContent(oHLTAddButton);
       },

       publishFilters: function () {
           console.log("publish Filters");
           let fd = this.byId("filterDialog").getModel().getData();
           let cont = JSON.stringify(fd);
           let xxx = btoa(cont);
           let cmd = "PublishFilters(\"" + xxx + "\")";
           this.getView().getViewData().mgr.SendMIR(cmd, this.eveFilter.fElementId, "FWWebGUIEventFilter");
       },

       setFilterEnabled: function(oEvent)
       {
           console.log("enable filter", oEvent.getParameter("selected"));
           let cmd = "SetFilterEnabled(\"" + oEvent.getParameter("selected") + "\")";
           let mgr = this.getView().getViewData().mgr;
           mgr.SendMIR(cmd, this.eveFilter.fElementId, "FWWebGUIEventFilter");      
       },

       handleModeSelect: function(oEvent)
       {
           console.log("handle mode select idx =", oEvent.getParameter("selectedIndex"));
       },

       reloadEveFilter: function (eveEl) {
           console.log("relaod fitler controlelr ", this.eveFilter.fElementId, eveEl.fElementId);
           this.byId("filterDialog").getModel().
               setData({ modelData: eveEl.collection, hltData: eveEl.HLT });
       },

       closeFilterDialog : function(){
        this.byId("filterDialog").close();
       }

});


   return EventFilterController;

});
