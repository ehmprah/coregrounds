/**
 * @file Event.js
 * Handles events
 */
const callbacks = {};

export default {

  fire() {
    let i, item, event, chain;
    for (event of arguments) {
      // console.log(`Event fired: "${event}"`);
      chain = callbacks[ event ];
      if (chain == undefined) return;
      for (i = 0; i < chain.length; i++ ) {
        item = chain[i];
        item.callback.apply( null, item.args )
      }
    }
  },

  on(event, callback) {
    // We allow multiple events for this setter
    if (event.constructor === Array ) {
      for (let e of event) {
        callbacks[e] = callbacks[e] || [];
        callbacks[e].push({
          callback: callback,
          args: Array.prototype.splice.call( arguments, 2 )
        });
      }
    } else {
      callbacks[event] = callbacks[event] || [];
      callbacks[event].push({
        callback: callback,
        args: Array.prototype.splice.call( arguments, 2 )
      });
    }
  },

  off: function(event) {
    callbacks[event] = [];
  },

};
