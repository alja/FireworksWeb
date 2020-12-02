sap.ui.define(['rootui5/eve7/controller/Summary.controller',
               'rootui5/eve7/lib/EveManager'
], function(SummaryController, EveManager) {
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

         var src = this.mgr.childs[0].childs[2].childs;
         for (var i = 0; i < src.length; i++) {
            if (src[i].fName == "Collections")
               src = src[i].childs;
         }

         return this.createSummaryModel([], src, "/");
      },
      addCollection: function (evt){
         var world = this.mgr.childs[0].childs;
         var last = world.length -1;
         var fw2gui = (world[last]);
         this.mgr.SendMIR("RequestAddCollectionTable()", fw2gui.fElementId,"FW2GUI");
      }
   });
});
