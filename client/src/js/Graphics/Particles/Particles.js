/**
 * @file
 * A recyclable particle object
 */
import chroma from 'chroma-js';
import { color } from '../Color';
import { jitter, randomInt } from '../../Util';
import Cache from '../Cache';
import ParticlePool from './ParticlePool';

const Particles = [];

// Get a chroma scale between red and black for the explosion particles
const scale = chroma.scale(['yellow', 'red', 'black']);

export default {

  draw(context) {
    Particles.forEach((particle) => {
      particle.draw(context);
    });
  },

  update(delta) {
    Particles.forEach((particle, index) => {
      // Update each particle
      particle.update(delta);
      // Remove stale ones
      if (particle.alpha <= 0) {
        // Return to pool
        ParticlePool.free(particle);
        // And delete from active particles
        Particles.splice(index, 1);
      }
    });
  },

  // TODO add mininum range for dust, it needs to start as a circle
  // TODO add color range of greys
  dust(x, y, playerIndex) {
    let particle;
    let sprite;
    let size;

    const drawParticle = (context, s) => {
      context.fillStyle = '#aaa';
      context.fillRect(10 - s, 10 - s, s, s);
    };

    // Create 10 particles
    for (let i = 0; i < 5; i++) {
      // Get a new particle from the pool
      particle = ParticlePool.get();

      // Get random movement vector
      particle.shoot(25);

      // Move the particle to the dead entity's position
      particle.x = x + 64 + jitter(10);
      particle.y = y + 64 + jitter(10);

      size = randomInt(10, 4);

      // Set the cache identifier
      particle.cacheKey = `dust_${playerIndex}_${size}`;

      // Get sprite or build it if this is the first time it's needed
      sprite = Cache.get(particle.cacheKey);
      if (sprite === false) {
        Cache.set(particle.cacheKey, 10, 10, drawParticle, size);
      }

      // Finally add the particle to our array
      Particles.push(particle);
    }
  },

  onHit(x, y, playerIndex, percentage) {
    // TODO add jitter!
    const amount = Math.ceil(percentage * 20);

    // TODO this should correspond with the damage dealt!!!
    let i;
    let size;
    let particle;
    let sprite;

    // Create 10 particles
    for (i = 0; i < amount; i++) {
      // Get a new particle from the pool
      particle = ParticlePool.get();

      // Get random movement vector
      particle.shoot(50);

      // Move the particle to the dead entity's position
      particle.x = x + 64 + jitter(4);
      particle.y = y + 64 + jitter(4);

      // Get a size for the particle at random
      size = randomInt(10, 4);

      // Set the cache identifier
      particle.cacheKey = `onhit_${playerIndex}_${size}`;

      // Get sprite or build it if this is the first time it's needed
      sprite = Cache.get(particle.cacheKey);
      if (sprite === false) {
        // eslint-disable-next-line
        Cache.set(particle.cacheKey, 10, 10, (context, playerIndex) => {
          context.fillStyle = color(0.5, playerIndex);
          context.fillRect(10 - size, 10 - size, size, size);
        }, playerIndex);
      }

      // Finally add the particle to our array
      Particles.push(particle);
    }
  },

  // @todo WE COULD MAYBE INCREASE THEIR SIZE AT FIRST, then decay?
  // @todo increase particle size for more randomness
  spriteExplosion(entity) {
    let i, x, y, dx, dy, particle, sprite, original, size, variant;

    // Add debris particles
    addDebrisParticles(entity.x + 48, entity.y + 48, 20);

    // Create 4x4 particles per sprite
    for (x = 0; x < 4; x++) {
      for (y = 0; y < 4; y++) {
        // Get a new particle from the pool
        particle = ParticlePool.get();

        // Get random movement vector
        particle.shoot(125, 0.8);

        // Move the particle to the dead entity's position
        particle.x = entity.x + 48;
        particle.y = entity.y + 48;

        // Set the cache identifier
        particle.cacheKey = `${entity.eid}_${entity.player_id}_${x}_${y}`;

        // Each particle will have its own portion of the initial sprite. We
        // try to get a cached version of the part and build it if necessary.
        sprite = Cache.get(particle.cacheKey);
        if (sprite === false) {
          // Get original sprite
          if (entity.type === 'Wall') {
            original = Cache.get(`unit_1_${entity.player_id}`);
          } else {
            original = Cache.get(`unit_${entity.eid}_${entity.player_id}`);
          }
          // Build the portion
          // eslint-disable-next-line
          Cache.set(particle.cacheKey, 32, 32, function(context, sprite) {
            dx = x * 32;
            dy = y * 32;
            context.drawImage(sprite, dx, dy, 32, 32, 0, 0, 32, 32);
          }, original);
        }

        // Finally add the particle to our array
        Particles.push(particle);
      }
    }
  },
};

function addDebrisParticles(x, y, amount) {
  let particle;
  let size;
  let variant;
  let sprite;
  // Create 10 particles
  for (let i = 0; i < amount; i++) {
    // Get a new particle from the pool
    particle = ParticlePool.get();

    // Get random movement vector
    particle.shoot(75);

    // Move the particle to the dead entity's position
    particle.x = x;
    particle.y = y;

    // Get a size for the particle at random
    size = randomInt(10, 4);
    variant = randomInt(1, 0);

    // Set the cache identifier
    particle.cacheKey = `explosion_${size}_${variant}`;

    // Get sprite or build it if this is the first time it's needed
    sprite = Cache.get(particle.cacheKey);
    if (sprite === false) {
      // eslint-disable-next-line
      Cache.set(particle.cacheKey, 10, 10, (context) => {
        context.fillStyle = scale(Math.random());
        context.fillRect(10 - size, 10 - size, size, size);
      });
    }

    // Finally add the particle to our array
    Particles.push(particle);
  }
}
