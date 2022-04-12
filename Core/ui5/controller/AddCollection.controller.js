sap.ui.define([
    "sap/ui/core/mvc/Controller",
    "sap/ui/model/json/JSONModel",
    "sap/ui/model/Sorter",
    "sap/m/Column",
    "sap/m/Button",
    "sap/m/Text"
], function (Controller, JSONModel, Sorter, Column, Button, Text) {
    "use strict";

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

            let pthis = this;
            let beginButton = new sap.m.Button('simpleDialogAcceptButton', { text: "AddEntry", press: function () { pthis.addCollection(); } });
            let endButton = new sap.m.Button('simpleDialogCancelButton', { text: "Close", press: function () { pthis.dialog.close(); } });

            this.dialog.setEndButton(endButton);
            this.dialog.setBeginButton(beginButton);
        },

        onFilterPost: function (oEvent) {
            var txt = oEvent.getParameter("query");
            let filter = new sap.ui.model.Filter([
                new sap.ui.model.Filter("purpose", sap.ui.model.FilterOperator.Contains, txt),
                new sap.ui.model.Filter("moduleLabel", sap.ui.model.FilterOperator.Contains, txt),
                new sap.ui.model.Filter("productInstanceLabel", sap.ui.model.FilterOperator.Contains, txt),
                new sap.ui.model.Filter("processName", sap.ui.model.FilterOperator.Contains, txt),
                new sap.ui.model.Filter("type", sap.ui.model.FilterOperator.Contains, txt)]
                , false);

            this.getTable().getBinding("items").filter(filter, "Applications");
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

            oTable.bindItems({
                path: dpath,
                template: new sap.m.ColumnListItem({
                    cells: [
                        new sap.m.Text({ text: "{purpose}" }),
                        new sap.m.Text({ text: "{moduleLabel}" }),
                        new sap.m.Text({ text: "{productInstanceLabel}" }),
                        new sap.m.Text({ text: "{processName}" }),
                        new sap.m.Text({ text: "{type}" })
                    ]
                })
            });
            oTable.addStyleClass("sapUiSizeCompact");
        },

        addCollection: function () {
            let tt = this.getView().byId("tbar");
            let si = tt.getSelectedKey();
            let table = this.getView().byId(si);

            var oSelectedItem = table.getSelectedItems();
            var item1 = oSelectedItem[0];
            if (item1) {
                var obj = item1.getBindingContext().getObject();
                console.log("SELECT ", item1.getBindingContext().getObject());
                let isEDM = (si == "ctable");
                this.getView().getViewData().sumCtrl.sendAddCollectionMIR(isEDM, obj);
            }
        },

    });

    return AddCollectionController;
});
