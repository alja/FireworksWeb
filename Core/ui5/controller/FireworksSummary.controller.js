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
            }
            if (src[i].fName == "Associations") {
               let x = src[i].childs;
               this.createSummaryModel(tgt, x, "/");
            }
         }
         return tgt;
      },
      addCollection: function (evt){
         var world = this.mgr.childs[0].childs;
         var last = world.length -1;
         var fw2gui = (world[last]);
         this.mgr.SendMIR("RequestAddCollectionTable()", fw2gui.fElementId,"FW2GUI");
      },

      showGedEditor: function(elementId) {

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
                  // ged.setTitle("EEE");
                  this.oModel.setProperty("/title", this.editorElement.fName + "(" + this.editorElement._typename + ")");

                  let label = new sap.m.Text({ text: "Qualitiy type " + this.editorElement.qtype });
                  label.addStyleClass("sapUiTinyMargin");
                  ged.addContent(label);
                  this.makeStringSetter(el.FilterExpr, "Filter", "SetFilterExpr");
                  console.log("======== ", ged.getContent());
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
       },
   });
});
