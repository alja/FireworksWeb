sap.ui.define([
    "sap/ui/core/mvc/Controller",
    'rootui5/eve7/lib/EveManager',
    "sap/ui/model/json/JSONModel",
    'sap/ui/core/UIComponent',
    "sap/m/FormattedText",
    "sap/m/Button",
    "sap/m/Input",
    "sap/ui/model/Sorter",
    "sap/m/Text",
    "sap/ui/table/Column",
    "sap/ui/layout/HorizontalLayout"
], function (Controller, EveManager, JSONModel, UIComponent, FormattedText, Button, mInput, Sorter, mText, tableColumn, HorizontalLayout) {
    "use strict";



    var TriggerTableController = Controller.extend("fw.controller.TriggerTable", {

        onInit: function () {

            let oModel = new sap.ui.model.json.JSONModel();
            this.getView().setModel(oModel);

            let data = this.getView().getViewData();
            if (data) {
                this.setupManagerAndViewType(data.eveViewerId, data.mgr);
            }
            else {
                UIComponent.getRouterFor(this).getRoute("TriggerTable").attachPatternMatched(this.onViewObjectMatched, this);
            }


        },
        onViewObjectMatched: function (oEvent) {
            let args = oEvent.getParameter("arguments");
            this.setupManagerAndViewType(EVE.$eve7tmp.eveViewerId, EVE.$eve7tmp.mgr);
            this.mgr.controllers[0].setToolbarExpandedAction(this);
            delete EVE.$eve7tmp;
        },

        setupManagerAndViewType: function (eveViewerId, mgr) {
            this.eveViewerId = eveViewerId;
            this.mgr = mgr;

            let eviewer = this.mgr.GetElement(this.eveViewerId);
            let sceneInfo = eviewer.childs[0];
            let sceneId = sceneInfo.fSceneId;
            this.mgr.RegisterController(this);
            this.mgr.RegisterSceneReceiver(sceneId, this);

            let scene = this.mgr.GetElement(sceneId);
            this.eveTrigger = scene.childs[0];

            this.setupTable();
            this.updateModel();
        },



        getCellText: function (value, filtered) {
            // console.log("getCellText", filtered);
            if (filtered == 1) {
                return "<span class='eveTableCellFiltered'>" + value + "</span>";
            }
            else {
                return "<span class='eveTableCellUnfiltered'>" + value + "</span>";
            }
        },

        setupTable: function () {
            let pthis = this;
            var oTable = this.getView().byId("triggerTable");
            oTable.bindAggregation("columns", "/columns", function (sId, oContext) {
                return new tableColumn(sId, {
                    label: "{label}",
                    sortProperty: "{columnName}",
                    template: new FormattedText({
                        htmlText: {
                            parts: [
                                { path: oContext.getProperty("columnName") },
                                { path: "r" }
                            ],
                            formatter: pthis.getCellText
                        }
                    }),
                    showFilterMenuEntry: true,
                });
            });

            // bind the Table items to the data collection
            let oBinding = oTable.bindRows({
                path: "/rows",
                sorter: [
                    new Sorter({
                        path: "r",
                        descending: true
                    })
                ]
            });


            oTable.attachEvent("columnPress", function (evt) {
                var col = evt.getParameters().column;
                let descend = false;
                let ch = this.getColumns()[col._index];
                let ar = ["n", "r", "a"];

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
        },

        updateModel: function () {
            let data = { "rows": [], "columns": [{ columnName: "n", label : "Name" }, { columnName: "r" , label : "Result"}, { columnName: "a" , label : "Average"}] };
            let n = this.eveTrigger.name.length;
            for (let i = 0; i < n; ++i) {
                data.rows.push({ "n": this.eveTrigger.name[i], "r": this.eveTrigger.result[i], "a": this.eveTrigger.average[i]+"%" });
            }
            
            this.getView().getModel().setData(data);
        },
/*
        onSceneCreate: function (element, id) {
            console.log("trigger onSceneCreate", id);
        },

        sceneElementChange: function (el) {
        },
*/
        endChanges: function (oEvent) {
            // console.log("trigger endChanges ", this.eveTrigger);
            this.updateModel();
        },

        onFilterChange: function(oEvent){
            var oValue = oEvent.getParameter("value");
            var oFilters = new sap.ui.model.Filter("n", "Contains", oValue);
            var oTable = this.getView().byId("triggerTable"); 
            oTable.getBinding("rows").filter(oFilters); 
        },

        switchSingle: function () {
            let oRouter = UIComponent.getRouterFor(this);
            EVE.$eve7tmp = { mgr: this.mgr, eveViewerId: this.eveViewerId };

            oRouter.navTo("TriggerTable", { viewName: this.mgr.GetElement(this.eveViewerId).fName });
        },

        swap: function () {
            this.mgr.controllers[0].switchViewSides(this.mgr.GetElement(this.eveViewerId));
        },

        detachViewer: function () {
            this.mgr.controllers[0].removeView(this.mgr.GetElement(this.eveViewerId));
            this.destroy();
        }
    });

    return TriggerTableController;
});
