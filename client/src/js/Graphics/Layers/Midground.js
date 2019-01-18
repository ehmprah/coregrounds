/**
 * @file Midground.js
 * Renders entities on their on layer.
 */
import Event from '../../Event';
import Cache from '../Cache';
import Game from '../../Game/Game';
import { drawStats, drawArea } from '../../Util/Debug';
import Config from '../../Config/Config';

let canvas;
let context;
Event.on('cache clear', () => {
  canvas = Cache.create('Midground', 1920, 1080);
  context = canvas.getContext('2d');
});

export default {
  draw(ctx) {
    ctx.drawImage(canvas, 0, 0);
  },
};

function update() {
  // Clear the midground.
  context.clearRect(0, 0, 1920, 1080);

  // Build array from entities.
  const entities = Object.values(Game.state.entities);

  // Iterate over array and draw cores and walls.
  entities.forEach((entity) => {
    if (entity.type === 'Wall') {
      entity.draw(context);
      entity.drawMeta(context);
    }
  });

  // Draw units afterwards
  entities.forEach((entity) => {
    if (
      entity.type === 'Tower' ||
      entity.type === 'Minion' ||
      entity.type === 'Trigger'
    ) {
      entity.draw(context);
    }
  });

  entities.forEach((entity) => {
    if (entity.type === 'Core') {
      entity.draw(context);
      entity.drawMeta(context);
    }
  });

  // And finally projectiles and unit meta
  entities.forEach((entity) => {
    if (entity.type === 'Projectile') {
      entity.draw(context);
    }
    if (entity.type === 'Tower' || entity.type === 'Minion') {
      entity.drawMeta(context);
    }
  });

  // Debug: draw entity stats.
  if (Config.User.get('debug-stats')) {
    entities.forEach((entity) => {
      drawStats(context, entity);
    });
  }

  // Debug: draw entity area.
  if (Config.User.get('debug-area')) {
    entities.forEach((entity) => {
      drawArea(context, entity);
    });
  }
}

Event.on('network update game', update);
