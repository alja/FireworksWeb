sap.ui.define(['rootui5/eve7/controller/Main.controller',
               'rootui5/eve7/lib/EveManager'
], function(MainController, EveManager) {
   "use strict";    

   return MainController.extend("custom.MyNewMain", {                    

      onInit: function() {
         // console.log('MAIN CONTROLLER INIT');
         MainController.prototype.onInit.apply(this, arguments);
         this.mgr.handle.SetReceiver(this);
      },
      

      OnWebsocketMsg : function(handle, msg, offset)
      {
         // console.log("helloOnWebsocketMsg GOT YOU  !!!!!! \n");
         this.mgr.OnWebsocketMsg(handle, msg, offset);
      },
      

      useConnection : function(handle)
      {
         this.handle = handle;
         this.mgr.handle = handle;

         handle.SetReceiver(this);
         handle.Connect();
      },   

      showHelp : function(oEvent) {
         alert("=====User support: fireworks@cern.ch");
      }
   });
});
