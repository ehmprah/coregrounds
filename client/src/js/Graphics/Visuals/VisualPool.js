/**
 * @file
 * Pooling for the Visuals
 */
import Visual from './Visual';

const Pool = {
  free: 0,
  pool: [],
};

export default {
  get() {
    if (Pool.free) {
      Pool.free -= 1;
    } else {
      Pool.pool.push(new Visual());
    }
    return Pool.pool.pop();
  },

  free(v) {
    Pool.free += 1;
    v.reset();
    Pool.pool.push(v);
  },
};
