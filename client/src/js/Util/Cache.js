/**
 * @file
 * A little helper class for caching
 */
import Event from '../Event';

let cache = {};

// Restore cached items after load
Event.on('after load', () => {
  const stored = window.localStorage.getItem('cgs-cache');
  if (stored) {
    cache = JSON.parse(stored);
  }
});

export default {
  get(key) {
    if (cache[key] !== undefined && cache[key].expire > Date.now()) {
      return cache[key].data;
    }
    return false;
  },

  set(key, data, lifetimeInSeconds) {
    cache[key] = {
      expire: Date.now() + (lifetimeInSeconds * 1000),
      data,
    };
    // Also save it in the local storage
    window.localStorage.setItem('cgs-cache', JSON.stringify(cache));
  },

  expire(key) {
    if (cache[key] !== undefined) {
      cache[key].expire = 0;
    }
  },
};
