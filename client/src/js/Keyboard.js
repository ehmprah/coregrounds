/**
 * @file
 * Simple keyboard shortcut syntax suger
 */
const callbacks = {};

export default {
  // Add an keyboard shortcut.
  on(keycode, callback, ...args) {
    // Create event if we don't have it yet
    callbacks[keycode] = callbacks[keycode] || [];
    // Add item to callbacks
    callbacks[keycode].push({ callback, args });
  },
};

function keyup(e) {
  const chain = callbacks[e.keyCode];
  if (chain !== undefined) {
    chain.forEach(item => item.callback.apply(null, item.args.concat([e])));
  }
}

document.addEventListener('keyup', keyup, false);
