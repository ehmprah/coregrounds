/**
 * @file
 * Pooling for all networked entities
 */
import Core from './Models/Units/Core';
import Wall from './Models/Units/Wall';
import Minion from './Models/Units/Minion';
import Tower from './Models/Units/Tower';
import Trigger from './Models/Units/Trigger';
import Projectile from './Models/Units/Projectile';

const Pools = {
  Core: {
    free: 0,
    pool: [],
  },
  Wall: {
    free: 0,
    pool: [],
  },
  Minion: {
    free: 0,
    pool: [],
  },
  Tower: {
    free: 0,
    pool: [],
  },
  Trigger: {
    free: 0,
    pool: [],
  },
  Projectile: {
    free: 0,
    pool: [],
  },
};

export default {
  get(type) {
    const pool = Pools[type];
    if (pool.free) {
      pool.free -= 1;
    } else {
      if (type === 'Core') pool.pool.push(new Core());
      if (type === 'Wall') pool.pool.push(new Wall());
      if (type === 'Minion') pool.pool.push(new Minion());
      if (type === 'Tower') pool.pool.push(new Tower());
      if (type === 'Trigger') pool.pool.push(new Trigger());
      if (type === 'Projectile') pool.pool.push(new Projectile());
    }
    return pool.pool.pop();
  },
  free(type, object) {
    const pool = Pools[type];
    pool.free += 1;
    object.reset();
    pool.pool.push(object);
  },
};
