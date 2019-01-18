/**
 * @file
 * A little helper class for caching
 */
const cache = {};

module.exports = {
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
  },

  expire(key) {
    if (cache[key] !== undefined) {
      cache[key].expire = 0;
    }
  },
};
