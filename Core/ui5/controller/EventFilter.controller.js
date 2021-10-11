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



    var EventFilterController = Controller.extend("fw.controller.EventFilter", {
        onInit: function () {
            this.eveFilter = this.getView().getViewData().gui;
            var oModel = new sap.ui.model.json.JSONModel();
            var pthis = this;
            oModel.setData({ modelData: this.eveFilter.collection, hltData: this.eveFilter.HLT, filterMode: this.eveFilter.filterMode });
            
                 oModel.attachPropertyChange( function(oEvent){
                     console.log("event: ", oEvent);
                     pthis.applyButton.setType(sap.m.ButtonType.Emphasized);
                  }, this); 
            
            this.byId("filterDialog").setModel(oModel);
        },

        openFilterDialog: function () {
            console.log("open filter dialog");
            this.byId("filterDialog").open();
        },

       makeModeGUI: function () {
            var oBtnGroup = new sap.m.RadioButtonGroup("filterModeGrp");
            oBtnGroup.setColumns(2);
            var dm = this.byId("filterDialog").getModel();
            oBtnGroup.attachSelect(function (oEvent) {
                let fm = oEvent.getParameter("selectedIndex") + 1;
                dm.setProperty("/filterMode", fm);
                dm.firePropertyChange();
            });

            let b1 = new sap.m.RadioButton({text:"OR"});
            oBtnGroup.addButton(b1);
            let b2 = new sap.m.RadioButton({text:"AND"});
            oBtnGroup.addButton(b2);

            var mhl = new sap.m.FlexBox({
                alignItems: "Start",
                direction: "Column",
                items: [
                    new sap.m.Label({ text: "Filter Mode:", class: "sapUiTinyMarginBottom", design: "Bold" }),
                    oBtnGroup
                ]
            });

            return mhl;
        },

        buildFilterGUI: function () {
            this.makePlainTable();
            this.makeHLTTable();

            let box = new sap.m.FlexBox({ direction: "Row" });

            let mg = this.makeModeGUI();
            mg.setLayoutData(new sap.m.FlexItemData({ growFactor: 1, shrinkFactor: 1, baseSize: "0%" }));
            this.setFilterModeFromEveElement();

            let dialog = this.byId("filterDialog");
            var pthis = this;

            let xl = new sap.m.Label({ text: "FilterStatus:", design: "Bold" });
            this.filterStatus = new sap.m.ObjectStatus("FilterStatusId", { class: "sapMObjectStatusLarge, sapUiLargeMargin", text: "NA" });
            this.filterIndicator = new sap.m.ProgressIndicator("ProgressIndicator",
                {
                    class: "sapUILargeMarginBottom",
                    percentValue: "50",
                    displayValue: "50 event of 100 event",
                    width: "300px"
                });

                this.processlabel = new sap.m.Label({text:"Abort", visible: false});
            let hso = new sap.ui.layout.VerticalLayout("StatusLayout", { width: "100%", content: [this.filterStatus, this.filterIndicator, this.processlabel] });
            let vlo = new sap.ui.layout.VerticalLayout({ class: "sapUiLargeMargin", width: "100%", content: [xl, hso] });
            vlo.setLayoutData(new sap.m.FlexItemData({ growFactor: 1, shrinkFactor: 1, baseSize: "0%" }));


            let disableButton = new sap.m.Button({
                text: "DisableFiltering", press: function () {
                    let mgr = pthis.getView().getViewData().mgr;
                    mgr.SendMIR("setFilterEnabled(0)", pthis.eveFilter.fElementId, "FWWebGUIEventFilter");
                }
            });

            let sxl = new sap.m.Label({ text: "Actions:", design: "Bold" });
            console.log("make APPLY BUTTON");
            this.applyButton = new sap.m.Button({
                text: "ApplyFilters", press: function () {
                    pthis.publishFilters();
                    setEnabled(false);
                }
            });
            this.setFilterStatusFromEveElement();
            if (this.eveFilter.statusID == 0)
                this.applyButton.setType(sap.m.ButtonType.Emphasized);
            let closeButton = new sap.m.Button({ text: "Hide", press: function () { pthis.closeFilterDialog(); } });
            let actionLayout = new sap.ui.layout.VerticalLayout({ content: [disableButton, this.applyButton] });

            let tal = new sap.ui.layout.VerticalLayout({ content: [sxl, actionLayout] });
            tal.setLayoutData(new sap.m.FlexItemData({ growFactor: 1, shrinkFactor: 1, baseSize: "0%" }));

            box.addItem(mg);
            box.addItem(vlo);
            box.addItem(tal);

            this.byId("filterDialog").addContent(box);
            dialog.addButton(closeButton);


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
                    model.oData["modelData"].splice(idx, 1);
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

        makeHLTTable: function () {
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
                    model.oData["hltData"].splice(idx, 1);
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

            this.byId("hltPanel").addContent(oHLTTable);
            this.byId("hltPanel").addContent(oHLTAddButton);
        },

        publishFilters: function () {
            let fd = this.byId("filterDialog").getModel().getData();
            console.log("publish Filters", fd);
            let cont = JSON.stringify(fd);
            let xxx = btoa(cont);
            let cmd = "PublishFilters(\"" + xxx + "\")";
            this.getView().getViewData().mgr.SendMIR(cmd, this.eveFilter.fElementId, "FWWebGUIEventFilter");
        },

        setFilterEnabled: function (oEvent) {
            console.log("enable filter", oEvent.getParameter("selected"));
            let cmd = "SetFilterEnabled(\"" + oEvent.getParameter("selected") + "\")";
            let mgr = this.getView().getViewData().mgr;
            mgr.SendMIR(cmd, this.eveFilter.fElementId, "FWWebGUIEventFilter");
        },

        refreshEveFilterResults: function (eveEl) {
            this.eveFilter = eveEl;
            this.applyButton.setType(sap.m.ButtonType.Default);
            this.byId("filterDialog").getModel().
                setData({ modelData: eveEl.collection, hltData: eveEl.HLT, filterMode: eveEl.filterMode });

            this.setFilterModeFromEveElement();
            this.setFilterStatusFromEveElement();
        },

        closeFilterDialog: function () {
            this.byId("filterDialog").close();
        },

        setFilterModeFromEveElement: function () {
            let data = this.byId("filterDialog").getModel().getData();
            data.filterMode = this.eveFilter.filterMode;
            if (this.eveFilter.filterMode) {
                let bg = sap.ui.getCore().byId("filterModeGrp");
                console.log("---set filter mode from eve ", bg);
                let idx = this.eveFilter.filterMode - 1;
                bg.getButtons()[idx].setSelected(true);
            }
        },

        setFilterStatusFromEveElement: function () {
            let os = this.filterStatus;
            console.log("os ", this.eveFilter);

            let per = 100 * this.eveFilter.NSelected / this.eveFilter.NTotal;


            // busy
            if (this.eveFilter.statusID == 3) {
                this.filterIndicator.setVisible(false);
                this.processlabel.setVisible(true);
                this.processlabel.setText(this.eveFilter.fTitle );
                this.applyButton.setEnabled(false);
            }
            else {
                this.filterIndicator.setVisible(true);
                this.processlabel.setVisible(false);
                this.applyButton.setEnabled(true);
            }

            this.filterIndicator.setPercentValue(per);
            this.filterIndicator.setDisplayValue(this.eveFilter.NSelected + " of " + this.eveFilter.NTotal + " events selected");
            this.filterIndicator.setEnabled(this.eveFilter.filterStatus);


            switch (this.eveFilter.statusID) {
                case 0:
                    os.setText("Filter Disabled");
                    os.setState(sap.ui.core.ValueState.Information);
                    os.setIcon("sap-icon://information");
                    break;
                case 1:
                    os.setText("Filter Enabled");
                    os.setState(sap.ui.core.ValueState.Success);
                    os.setIcon("");
                    break;
                case 2:
                    os.setText("Filter Withdrawn");
                    os.setState(sap.ui.core.ValueState.Error);
                    os.setIcon("sap-icon://error");
                    break;
                case 3:
                    os.setText("Filter Busy ");// + this.eveFilter.fTitle );
                    os.setState(sap.ui.core.ValueState.Warning);
                    os.setIcon("sap-icon://warning");
                    break;
                default:
                    os.setText("Unknown");

            }
        }

    });


    return EventFilterController;

});
