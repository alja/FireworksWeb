sap.ui.define([
   'sap/ui/core/UIComponent', 'sap/ui/core/mvc/XMLView', 'sap/m/routing/Router' /* Router is only needed for packaging */
], function(UIComponent, XMLView) {
   "use strict";

   var Component = UIComponent.extend("fw.Component", {
      metadata : {
         manifest: "json"
      },
      init: function() {
         UIComponent.prototype.init.apply(this, arguments);
         if (this.getRouter()) {
            this.getRouter().initialize();
         }
      }
   });

   return Component;

});
