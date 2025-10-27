sap.ui.define([
    "sap/ui/core/mvc/Controller",
    "sap/ui/core/CustomData",
    "sap/ui/model/json/JSONModel",
    "sap/ui/model/Sorter",
    "sap/m/Column",
    "sap/m/Button",
    "sap/m/Text"
], function (Controller, CustomData, JSONModel, Sorter, Column, Button, Text) {
    "use strict";


    Text.extend("GhostText", {
        renderer: {},
        onAfterRendering: function () {
            if (Text.prototype.onAfterRendering) {
                Text.prototype.onAfterRendering.apply(this, arguments);
            }
            let bad = this.getParent().mAggregations.customData[0].getValue("empty");
            if (bad) {
                this.$().css("color", "lightgray");
                var elm = this.$().parent();

                elm.css("color", "lightgray");
            }
        }
    });


    var AddCollectionController = Controller.extend("fw.controller.AddCollection", {

        onInit: function () {
            let oModel = new sap.ui.model.json.JSONModel();
            // set the data for the model
            oModel.setData(this.getView().getViewData().d);
            this.getView().setModel(oModel);

            this.setTable("ctable", "/c");
            this.setTable("atable", "/a");

            let data = this.getView().getViewData().d;
            this.getView().byId("ctab").setCount(data["c"].length);
            this.getView().byId("atab").setCount(data["a"].length);

            this.getView().setModel(oModel);

            this.dialog = this.getView().byId("acdialog");
        },

        onFilterPost: function (oEvent) {
            this.applyFilter();
        },

        showEmptyChange : function (oEvent){
            console.log("show empty = ", this.getView().byId("empty").getSelected());
            this.applyFilter();
        },

        applyFilter: function()
        {
            let showEmpty = this.getView().byId("empty").getSelected();
            let txt = this.getView().byId("csr").getValue();

            let colFilter = new sap.ui.model.Filter([
                new sap.ui.model.Filter("purpose", sap.ui.model.FilterOperator.Contains, txt),
                new sap.ui.model.Filter("moduleLabel", sap.ui.model.FilterOperator.Contains, txt),
                new sap.ui.model.Filter("productInstanceLabel", sap.ui.model.FilterOperator.Contains, txt),
                new sap.ui.model.Filter("processName", sap.ui.model.FilterOperator.Contains, txt),
                new sap.ui.model.Filter("type", sap.ui.model.FilterOperator.Contains, txt)]
                , false);

            if (showEmpty)
            {
                this.getTable().getBinding("items").filter(colFilter, "Applications");
            }
            else
            {
                let tf = new sap.ui.model.Filter({filters: [colFilter, new sap.ui.model.Filter("bad", sap.ui.model.FilterOperator.EQ, false)], and:true});
                this.getTable().getBinding("items").filter(tf, "Applications");
            }

        },

        getTable: function()
        {
            let tt = this.getView().byId("tbar");
            return this.getView().byId(tt.getSelectedKey());
        },

        setTable: function (tId, dpath) {
            var oTable = this.getView().byId(tId);

            oTable.setIncludeItemInSelection(true);
            oTable.bActiveHeaders = true;

            oTable.attachEvent("columnPress", function (evt) {
                var col = evt.getParameters().column;
                let descend = false;
                let ch = this.getColumns()[col._index];
                let ar = ["purpose", "moduleLabel", "productInstanceLabel", "processName", "type"];

                // init first time ascend
                if (col.getSortIndicator() == sap.ui.core.SortOrder.Descending || col.getSortIndicator() == sap.ui.core.SortOrder.None) {
                    descend = true;
                }
                else {
                    descend = false;
                }
                // flip the state
                descend = !descend;
                var oSorter = new Sorter(ar[col._index - 1], descend);
                var oItems = this.getBinding("items");
                oItems.sort(oSorter);

                var indicator = descend ? sap.ui.core.SortOrder.Descending : sap.ui.core.SortOrder.Ascending;
                col.setSortIndicator(indicator);
            });

            var oColumnListTemplate = new sap.m.ColumnListItem({
                cells: [
                    new GhostText({ text: "{purpose}" }),
                    new GhostText({ text: "{moduleLabel}" }),
                    new GhostText({ text: "{productInstanceLabel}" }),
                    new GhostText({ text: "{processName}" }),
                    new GhostText({ text: "{type}" })
                ],
                customData: { "key": "empty", "value": "{bad}" }
            });
            var oDataTemplate = new CustomData({key:"bad", value: "{bad}"});
            oColumnListTemplate.addCustomData(oDataTemplate);
            oTable.bindAggregation("items", dpath, oColumnListTemplate);


            oTable.addStyleClass("sapUiSizeCompact");
        },

        addCollection: function (oEvent) {
            let tt = this.getView().byId("tbar");
            let si = tt.getSelectedKey();
            let table = this.getView().byId(si);
            let iname = this.getView().byId("nameInput");

            var oButton = oEvent.getSource();
            var sButtonText = oButton.getText();

            var oSelectedItem = table.getSelectedItems();
            var item1 = oSelectedItem[0];
            if (item1) {
                var obj = item1.getBindingContext().getObject();
                // console.log("SELECT ", item1.getBindingContext().getObject());
                let isEDM = (si == "ctable");
                obj.customDisplayName = iname.getValue();
                let summaryController = this.getView().getViewData().sumCtrl;
                summaryController.sendAddCollectionMIR(isEDM, obj);

                if (sButtonText !== "AddData") {
                    this.dialog.close();
                    iname.setValue("");
                }
            }

        },
        onClose: function() {
            this.dialog.close();
        },
        onResetName: function() {
            let iname = this.getView().byId("nameInput");
            iname.setValue("");
        }

    });

    return AddCollectionController;
});
