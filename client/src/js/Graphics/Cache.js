/**
 * Cache.js
 * The allmighty cache library!
 *
 * TODO rework this the following way: remove set callback. all caches will be created on load
 * event, and for drawing we always use the update method, that's the most clean way.
 * TODO remove the reset thing
 */
import Event from '../Event';

let canvas = {};

Event.on('app loaded', () => {
  Event.fire('cache clear');
});

Event.on('game over', () => {
  canvas = {};
  Event.fire('cache clear');
});

export default {

  create(key, width, height) {
    if (createCanvas(key, width, height)) {
      return canvas[key];
    }
  },

  get(key) {
    if (canvas[key] === undefined) {
      return false;
    }
    return canvas[key];
  },

  set(key, width, height, fn, ...args) {
    if (createCanvas(key, width, height)) {
      // Add the context to drawing function params
      args.unshift(canvas[key].getContext('2d'));
      // Fire the callback
      fn(...args);
      // Return the canvas for immediate drawing
      return canvas[key];
    }
  },

  update(key, fn, ...args) {
    if (canvas[key] === undefined) {
      throw new Error(`Trying to update undefined cache at: ${key}`);
    }
    // Add the context to drawing function params
    args.unshift(canvas[key].getContext('2d'));
    // Fire the callback
    fn(...args);
  },
};

function createCanvas(key, width, height) {
  if (canvas[key] !== undefined) {
    // eslint-disable-next-line
    console.warn(`Trying to set already defined cache at: ${key}`);
    return false;
  }
  canvas[key] = document.createElement('canvas');
  canvas[key].width = width;
  canvas[key].height = height;
  return true;
}
