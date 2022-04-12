sap.ui.define(['rootui5/eve7/controller/Summary.controller',
               'rootui5/eve7/lib/EveManager',
               "sap/ui/layout/SplitterLayoutData",
               "rootui5/eve7/controller/Ged.controller",
               "sap/ui/core/mvc/XMLView"
], function(SummaryController, EveManager, SplitterLayoutData, GedController, XMLView) {
   "use strict";

   return SummaryController.extend("fw.FireworksSummary", {

      onInit: function() {
         SummaryController.prototype.onInit.apply(this, arguments);

         var oTree = this.getView().byId("tree");
           this.expandLevel = 0;
      },
      createModel: function() {
         // this is central method now to create summary model
         // one could select top main element which will be shown in SummaryView

         this.summaryElements = {};

         let tgt = [];

         var src = this.mgr.childs[0].childs[2].childs;
         for (var i = 0; i < src.length; i++) {
            if (src[i].fName == "Collections") {
               let x  = src[i].childs;
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
      },
      addCollection: function (evt) {
         var world = this.mgr.childs[0].childs;
         var last = world.length - 1;
         var fw2gui = (world[last]);

         if (!this.acGUI) {
            this.mgr.SendMIR("RequestAddCollectionTable()", fw2gui.fElementId, "FW2GUI");
         }
         else {
            this.acGUI.open();
         }
      },

      initAddCollectionEditor: function (msg, fw2gui) {
         let pthis = this;
         XMLView.create({
            viewName: "fw.view.AddCollection",
            viewData: { d: msg.arr, sumCtrl: pthis }
         }).then(function (oView) {
            pthis.acGUI = oView.getController().dialog;
            pthis.acGUI.open();
         });
      },

      sendAddCollectionMIR(isEDM, obj) {
         var fcall = "AddCollection(" + isEDM + ",\"" + obj.purpose + "\", \"" + obj.moduleLabel + "\", \"" + obj.productInstanceLabel + "\", \"" + obj.processName + "\", \"" + obj.type + "\")";

         var world = this.mgr.childs[0].childs;
         var last = world.length - 1;
         var fw2gui = (world[last]);
         this.mgr.SendMIR(fcall, fw2gui.fElementId, "FW2GUI");


         this.rebuild = true;
         this.acGUI.close();
      },

      showGedEditor: function (elementId) {

         var sumSplitter = this.byId("sumSplitter");

         if (!this.ged) {
            var pthis = this;

            XMLView.create({
               viewName: "rootui5.eve7.view.Ged",
               layoutData: new SplitterLayoutData("sld", { size: "30%" }),
               height: "100%"
            }).then(function (oView) {
               pthis.ged = oView;
               pthis.ged.getController().setManager(pthis.mgr);
               pthis.ged.getController().buildFWEveAssociationSetter = function (el) {
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
               };


               pthis.ged.getController().showGedEditor(sumSplitter, elementId);

            });
         } else {
            this.ged.getController().showGedEditor(sumSplitter, elementId);
         }
      }
   });
});
