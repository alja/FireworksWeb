sap.ui.define(['rootui5/eve7/controller/Main.controller',
               'rootui5/eve7/lib/EveManager'
], function(MainController, EveManager) {
   "use strict";    

   return MainController.extend("custom.MyNewMain", {                    

      onInit: function() {
                       MainController.prototype.onInit.apply(this, arguments);
      },
            
      showHelp : function(oEvent) {
         alert("=====User support: fireworks@cern.ch");
      }
   });
});
