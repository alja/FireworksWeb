sap.ui.define(['rootui5/eve7/controller/Summary.controller',
   'rootui5/eve7/lib/EveManager',
   "sap/ui/layout/SplitterLayoutData",
   "rootui5/eve7/controller/Ged.controller",
   "sap/ui/core/mvc/XMLView",
   "sap/ui/model/json/JSONModel",
   "sap/m/CustomTreeItem",
   "sap/m/FlexBox",
   "sap/m/CheckBox",
   "sap/m/Text",
   "sap/m/Button"
], function (SummaryController, EveManager, SplitterLayoutData, GedController, XMLView, JSONModel, CustomTreeItem,
   FlexBox, mCheckBox, mText, mButton) {
   "use strict";

   let FWSummaryCustomItem = CustomTreeItem.extend('rootui5.eve7.lib.EveSummaryCustomItem', {
      renderer: {},

      metadata: {
         properties: {
            elementId: 'string',
            background: 'string',
            mainColor: 'string',
            showCheckbox: 'boolean',
            showRnrChildren: 'boolean'
         }
      },

      onAfterRendering: function () {
         let flex = this.getContent()[0].getItems()[1];
         let btn = flex.getItems()[1]; // set color on the second button
         btn.$().css('background-color', this.getMainColor());
      }

   });


   return SummaryController.extend("fw.FireworksSummary", {

      onInit: function () {

         let data = [{ fName: "Event" }];

         this.summaryElements = {}; // object with all elements, used for fast access to elements by id

         let oTree = this.getView().byId("tree");

         let oModel = new JSONModel();
         oModel.setData([]);
         oModel.setSizeLimit(10000);
         oModel.setDefaultBindingMode("OneWay");
         this.getView().setModel(oModel, "treeModel");

         let oItemTemplate = new FWSummaryCustomItem({
            content: [
               new FlexBox({
                  width: "100%",
                  alignItems: "Start",
                  justifyContent: "SpaceBetween",
                  items: [
                     new FlexBox({
                        alignItems: "Start",
                        items: [
                           new mCheckBox({ visible: "{treeModel>fShowCheckbox}", selected: "{treeModel>fSelected}", select: this.clickItemSelected.bind(this) }),
                           new mText({ text: " {treeModel>fName}", tooltip: "{treeModel>fTitle}", renderWhitespace: true, wrapping: false })
                        ]
                     }),
                     new FlexBox({
                        alignItems: "End",
                        items: [
                           new mButton({ id: "errBtn", visible: "{treeModel>has_error}", icon: "sap-icon://alert", type: "Transparent", press: this.showError.bind(this) }),
                           new mButton({ id: "detailBtn", visible: "{treeModel>fShowButton}", icon: "sap-icon://edit", type: "Transparent", tooltip: "Show editor", press: this.pressGedButton.bind(this), tooltip: "Show Collection Editor" }),
                           new mButton({ id: "childs", visible: "{treeModel>fShowButton}", icon: "sap-icon://arrow-down", type: "Transparent", press: this.pressItemsGedButton.bind(this), tooltip:"Show Collection Items Editor" })
                        ]
                     })
                  ]
               })
            ],

            elementId: "{treeModel>fElementId}",
            mainColor: "{treeModel>fMainColor}",
            showCheckbox: "{treeModel>fShowCheckbox}",
            showRnrChildren: "{treeModel>fShowRnrChildren}"
         });

         oItemTemplate.addStyleClass("eveSummaryItem");
         oItemTemplate.attachBrowserEvent("mouseenter", this.onMouseEnter, this);
         oItemTemplate.attachBrowserEvent("mouseleave", this.onMouseLeave, this);
         oTree.bindItems("treeModel>/", oItemTemplate);

         this.template = oItemTemplate;

         this.rebuild = false;
         this.expandLevel = 0;

         this.customDisplayNames = {};
         console.log("Initialized customDisplayNames: ", this.customDisplayNames)

         // depth
         this.frontLayerDepth = -100;
         let treeModel = this.getView().getModel("treeModel");
         treeModel.attachPropertyChange(this.onTreeModelChange.bind(this));
      },

      onTreeModelChange: function(oEvent) {
         let sPath = oEvent.getParameter("path");
         let sProperty = sPath.split("/").pop();
         let bNewValue = oEvent.getParameter("value");
         
         // Check if a selection changed to true
         if (sProperty === "fSelected" && bNewValue === true) {
            let sElementPath = sPath.replace("/fSelected", "");
            let oData = this.getView().getModel("treeModel").getProperty(sElementPath);
             
            if (oData && oData.fElementId) {
               this.bringToFront(oData.fElementId);
            }
         }
      },
     
      bringToFront: function(elementId) {
         this.frontLayerDepth -= 10;
         this.mgr.SendMIR("SetCurrentDepth(" + this.frontLayerDepth + ")", elementId, "REveElement");
      },

      createSummaryModel: function (tgt, src, path) {
         console.log("createSummaryModel processing src:", src);

         for (let n = 0; n < src.length; ++n) {
            let elem = src[n];

            let newelem = { fName: elem.fName, fTitle: elem.fTitle || elem.fName, id: elem.fElementId, fHighlight: "None", fBackground: "", fMainColor: "", fSelected: false };

            newelem.has_error = false;
            if (typeof elem.err != 'undefined' && elem.err !== "")
               newelem.has_error = true;

            this.setElementsAttributes(newelem, elem);

            newelem.path = path + n;
            newelem.masterid = elem.fMasterId || elem.fElem

<<<<<<< HEAD
            // console.log("newelem.fName = ", newelem.fName);
=======
            console.log("newelem.fName = ", newelem.fName);
>>>>>>> 316de5b (layer settings)
            tgt.push(newelem);

            this.summaryElements[newelem.id] = newelem;

            // do not show items as children
            //if ((elem.childs !== undefined) && this.anyVisible(elem.childs))
            //   newelem.childs = this.createSummaryModel([], elem.childs, newelem.path + "/childs/");
         }

         return tgt;
      },

      showError: function (oEvent) {
         let item = oEvent.getSource().getParent().getParent().getParent();
         let ee = this.mgr.GetElement(item.getElementId());
         sap.m.MessageBox.error(ee.err);
      },

      pressGedButton: function (oEvent) {
         let item = oEvent.getSource().getParent().getParent().getParent();
         this.showGedEditor(item.getElementId(), true);
      },

      pressItemsGedButton: function (oEvent) {
         let item = oEvent.getSource().getParent().getParent().getParent();
         let elem = this.mgr.GetElement(item.getElementId());
         this.showGedEditor(elem.childs[0].fElementId, false);
      },

      createModel: function () {
         console.log("createModel called, customDisplayNames: ", this.customDisplayNames)
         
         let debug = 0;

         if (debug) {
            this.summaryElements = {};
            let src = this.mgr.childs;
            let result = this.createSummaryModel([], src, "/");
            return result;
         }
         else {
            this.summaryElements = {};

            let tgt = [];

            var src = this.mgr.childs[0].childs[2].childs;
            for (var i = 0; i < src.length; i++) {
               if (src[i].fName == "Collections") {
                  let x = src[i].childs;
                  this.createSummaryModel(tgt, x, "/");

                  this.mgr.RegisterSceneReceiver(src[i].fElementId, this);
               }
               if (src[i].fName == "Associations") {
                  if (src[i].childs)
                     this.createSummaryModel(tgt, src[i].childs, "/");
                  this.mgr.RegisterSceneReceiver(src[i].fElementId, this);
               }
            }

            return tgt;
         }
      },
      addCollection: function (evt) {
         var world = this.mgr.childs[0].childs;
         var last = world.length - 1;
         var fw2gui = (world[last]);

         if (!this.acGUI) {
            this.mgr.SendMIR("RequestAddCollectionTable()", fw2gui.fElementId, "FW2GUI");
            let btn = this.byId("addCollection");
            btn.setEnabled(false);
            btn.setText("Add Collections Processing ...");
         }
         else {
            this.acGUI.open();
         }
      },

      initAddCollectionEditor: function (msg, fw2gui) {
         let pthis = this;
         let btn = this.byId("addCollection");
         XMLView.create({
            viewName: "fw.view.AddCollection",
            viewData: { d: msg.arr, sumCtrl: pthis }
         }).then(function (oView) {
            pthis.acGUI = oView.getController().dialog;
            pthis.acGUI.open();
         });
         btn.setText("Add Collections");
         btn.setEnabled(true);
      },

      sendAddCollectionMIR(isEDM, obj) {
         //console.log("Send add collection MIR");
         var fcall = "AddCollection(" + isEDM + ",\"" + obj.purpose + "\", \"" + obj.moduleLabel + "\", \"" + obj.productInstanceLabel + "\", \"" + obj.processName + "\", \"" + obj.type + "\", \"" + obj.customDisplayName + "\")";

         let elObj = this.summaryElements;
         let isDuplicate = false;

         Object.entries(elObj).forEach(([key, value]) => {
         let elName = value.fName;
         
         if ( elName == obj.customDisplayName && isDuplicate === false) {
            isDuplicate = true;
            sap.m.MessageBox.error("Collection with this name has already been added!");
         }
         });

         if (isDuplicate === false) {
            console.log("sendibnfg add collection MIR");
            var world = this.mgr.childs[0].childs;
            var last = world.length - 1;
            var fw2gui = (world[last]);
            this.mgr.SendMIR(fcall, fw2gui.fElementId, "FW2GUI");

            this.rebuild = true;
         }
      },

      showGedEditor: function (elementId, showEDMInfo) {

         var sumSplitter = this.byId("sumSplitter");

         if (!this.ged) {
            var pthis = this;

            XMLView.create({
               viewName: "fw.view.FireworksGed",
               layoutData: new SplitterLayoutData("sld", { size: "30%" }),
               height: "100%"
            }).then(function (oView) {
               pthis.ged = oView;
               pthis.ged.getController().setManager(pthis.mgr);
               pthis.ged.getController().showFWGedEditor(sumSplitter, elementId, showEDMInfo);
            });
         } else {
            this.ged.getController().showFWGedEditor(sumSplitter, elementId, showEDMInfo);
         }
      }
   });
});