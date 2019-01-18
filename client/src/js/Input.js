/**
 * @file
 * The nitty gritty of user interaction.
 *
 * TODO We no longer need the fastclick library since "Chrome 32+ on Android
 * with width=device-width in the viewport meta tag doesn't have a 300ms delay"
 */
import Event from './Event';
import { throttle, pointToTile } from './Util';

const callbacks = {};
let x = 0, y = 0;

export default {

  // Get the current position as a tile
  tile: function() {
    return pointToTile(x, y);
  },

  // Get the current position as point
  position: function() {
    return { x: x, y: y };
  },

  getCurrentTile: function() {
    return [Math.floor(this.mouse.y / 128), Math.floor(this.mouse.x / 128)];
  },

  // Add an event handler.
  on: function(box, event, callback) {
    // Create event if we don't have it yet
    callbacks[event] = callbacks[event] || [];
    // Catch any additional args given
    let args = Array.prototype.splice.call(arguments, 3);
    // Add item to callbacks
    callbacks[event].push({ callback: callback, box: box, args: args });
  },

  // Remove an event handler.
  off: function( box, event, callback ) {
    let i;
    const chain = callbacks[event];
    if (chain === undefined) return;
    for ( i = 0; i < chain.length; i++ ) {
      if ( box == chain[i]['box'] ) {
        chain.splice( i, 1 );
      }
    }
  },
};

// Private functions
function move( e ) {
  // Set necessary variables
  e = e || window.event;
  var element = document.getElementById('coregrounds');
  var width = element.offsetWidth;
  var offset = element.getBoundingClientRect();
  var chain, length, item;

  // Get current cursor position
  x = e.pageX - offset.left;
  y = e.pageY - offset.top;
  if ( e.type == 'touchstart' ||
       e.type == 'touchmove' ||
       e.type == 'touchend' ) { console.log(e);
    // @todo check if this is working!
    x = e.touches[0].pageX - offset.left;
    y = e.touches[0].pageY - offset.top;
    // if ( typeof e.originalEvent.changedTouches !== undefined ) {
    //   x = e.originalEvent.changedTouches[0].pageX - offset.left;
    //   y = e.originalEvent.changedTouches[0].pageY - offset.top;
    // }
  }

  // Scale the cursor position if necessary
  if ( width < 1920 ) {
    x = x * 1920 / width;
    y = y * 1920 / width;
  }

  // We want ints instead of those nasty floats
  x = x | 0;
  y = y | 0;

  // Process mouseout events
  chain = callbacks['mouseout'];
  if (chain !== undefined) {
    length = chain.length;
    while (length--) {
      item = chain[length];
      // Only check position for boxes which are hovered over
      if (item.box.hover) {
        if (!cursorInBox(item.box) || !item.box.listening) {
          item.callback.apply(null, item.args);
        }
      }
    }
  }

  // Process mousein events
  chain = callbacks['mousein'];
  if (chain !== undefined) {
    length = chain.length;
    while (length--) {
      item = chain[length];
      // Only fire callback for boxes which are listening for input
      if (item.box.listening) {
        if (cursorInBox(item.box)) {
          item.callback.apply(null, item.args);
        }
      }
    }
  }
}

function cursorInBox(box) {
  if (x > box.x &&
      x < box.x + box.width &&
      y > box.y &&
      y < box.y + box.height) {
    return true;
  }
  return false;
}

function mousedown(e) {
  e = e || window.event;
  e.preventDefault();
  // Disable right clicks
  if (e instanceof MouseEvent) {
    if (e.button !== 0) return;
  }
  let item;
  const chain = callbacks['mousedown'];
  if (chain !== undefined) {
    length = chain.length;
    while (length--) {
      item = chain[length];
      // Only fire callback for boxes which are listening for input
      if (item.box.listening) {
        if (cursorInBox(item.box)) {
          item.callback.apply(null, item.args.concat([e]));
        }
      }
    }
  }
}

function mouseup(e) {
  e = e || window.event;
  // Disable right clicks
  if (e instanceof MouseEvent) {
    if (e.button !== 0) return;
  }
  let item;
  const chain = callbacks['mouseup'];
  if (chain !== undefined) {
    length = chain.length;
    while (length--) {
      item = chain[length];
      // Only fire callback for boxes which are listening for input
      if (item.box.listening) {
        if (cursorInBox(item.box)) {
          // Stop propagation for events which return false
          if (item.callback.apply(null, item.args.concat([e])) === false) return;
        }
      }
    }
  }
}

// Add event listeners once everything's ready
Event.on('after load', () => {
  // Once initially and on window size or orientation changes, we resize
  resize();
  const throttledResize = throttle(resize, 100);
  window.addEventListener('resize', throttledResize, false);
  window.addEventListener('orientationchange', throttledResize, false);

  // Listen for interaction with the canvas
  const canvas = document.getElementById('coregrounds');
  canvas.addEventListener('touchstart', mousedown);
  canvas.addEventListener('mousedown', mousedown);
  canvas.addEventListener('touchend', mouseup);
  canvas.addEventListener('mouseup', mouseup);

  // // Listen for mouse/finger movement
  const throttledMouseMove = throttle(move, 50);
  document.addEventListener('mousemove', throttledMouseMove);
  document.addEventListener('touchmove', throttledMouseMove);

  // Deactivate right click
  document.addEventListener('contextmenu', e => e.preventDefault());
});

function resize() {
  // Resize the game area
  var gameArea = document.getElementById('wrapper');
  var widthToHeight = 16/9;
  var newWidth = window.innerWidth;
  var newHeight = window.innerHeight;
  var newWidthToHeight = newWidth / newHeight;

  if (newWidthToHeight > widthToHeight) {
    newWidth = newHeight * widthToHeight;
    gameArea.style.height = newHeight + 'px';
    gameArea.style.width = newWidth + 'px';
  } else {
    newHeight = newWidth / widthToHeight;
    gameArea.style.width = newWidth + 'px';
    gameArea.style.height = newHeight + 'px';
  }

  gameArea.style.marginTop = (-newHeight / 2) + 'px';
  gameArea.style.marginLeft = (-newWidth / 2) + 'px';

  // Resize the ambient background canvases
  for (let item of document.querySelectorAll('.ambient')) {
    item.style.height = window.innerHeight + 'px';
    item.style.marginLeft = (-1920 + window.innerWidth)/2 + 'px';
  }

  Event.fire('resize');
}
