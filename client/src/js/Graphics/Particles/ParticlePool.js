/**
 * @file
 * Pooling for the particles
 */
import Particle from './Particle';

const Pool = {
  free: 0,
  pool: [],
};

export default {
  get() {
    if (Pool.free) {
      Pool.free -= 1;
    } else {
      Pool.pool.push(new Particle());
    }
    return Pool.pool.pop();
  },
  free(particle) {
    Pool.free += 1;
    particle.reset();
    Pool.pool.push(particle);
  },
};
