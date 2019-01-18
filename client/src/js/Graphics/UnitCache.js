/**
 * @file
 * Handles rendering for all entities and factories
 */
import { color } from './Color';
import Cache from './Cache';
import Config from '../Config/Config';
import Game from '../Game/Game';

export default {

  get(id, playerIndex, type, prefix = 'unit') {
    // Build cache key
    const key = `${prefix}_${id}_${playerIndex}`;
    const cached = Cache.get(key);
    if (cached) {
      return cached;
    }

    // Try to create the cache and subsequently do drawing on it
    if (Cache.create(key, 128, 128)) {
      // Create options object
      const options = {
        id,
        color: color(1, playerIndex),
        size: 128,
        angle: 0,
        base: 128,
        shadow: false,
      };

      if (type === 'Minion') {
        options.size = prefix === 'interface' || prefix === 'interface_limit'
          ? 80
          : 60;

        if (prefix === 'unit' || prefix === 'foremost') {
          options.angle = Math.PI / 2;
        }
      }

      if (type === 'Tower') {
        options.size = 85;
        if (prefix === 'unit') {
          options.angle = Math.PI / 2;
        }
        // Draw wall below towers
        if (prefix === 'interface') {
          Cache.update(key, (context, c) => {
            context.unitSprite(skinify(1, playerIndex), c);
          }, options.color);
        }
      }

      // For abilities we add a simple circle
      if (type === 'Ability') {
        options.size = 100;
        options.color = color(1, playerIndex);
        if (prefix === 'interface') {
          Cache.update(key, (context, p) => {
            context.beginPath();
            context.arc(64, 64, 56, 0, 2 * Math.PI, false);
            context.lineWidth = 4;
            context.fillStyle = '#222222';
            context.fill();
            context.strokeStyle = '#000';
            context.stroke();
            context.beginPath();
            context.arc(64, 64, 50, 0, 2 * Math.PI, false);
            context.lineWidth = 8;
            context.strokeStyle = '#333';
            context.stroke();
            context.arc(64, 64, 56, 0, 2 * Math.PI, false);
            context.fillStyle = color(0.1, p);
            context.fill();
          }, playerIndex);
        }
      }

      if (type === 'Projectile') {
        options.shadow = true;
        // TODO remove this hardcoded bullshit later
        if (id === 1214) {
          options.color = Config.COLOR_HEALING;
        }
      }

      // The base sprite which we ALWAYS render
      Cache.update(key, (context, o) => {
        // Add drop shadow for towers
        if (type === 'Tower' || type === 'Minion') {
          context.shadowColor = '#000';
          context.shadowBlur = 8;
          context.shadowOffsetX = 0;
          context.shadowOffsetY = 0;
        }

        context.unitSprite(
          skinify(o.id, playerIndex),
          o.color,
          o.size,
          o.angle,
          o.base,
          o.shadow
        );

        if (prefix === 'foremost' || prefix === 'interface_limit') {
          // For foremost minions we add a red tint
          setTimeout(() => {
            // Only paint on overlap with current shape
            context.resetTransform();
            context.globalCompositeOperation = 'source-atop';
            context.fillStyle = 'rgba(255, 0, 0, 0.25)';
            context.fillRect(0, 0, 128, 128);
          }, 0);
        }
      }, options);

      // Return the updated cache now
      return Cache.get(key);
    }
  },

  getHighlight(id, playerIndex, type, prefix = 'unit') {
    const key = `${prefix}_${id}_${playerIndex}`;
    let cached = Cache.get(`shadowed_${key}`);
    if (cached) {
      return cached;
    }
    // Get cached version without shadow
    cached = this.get(id, playerIndex, type, prefix);
    const highlighted = Cache.create(`shadowed_${key}`, 128, 128);
    // Draw the sprite asynchronously to avoid empty caches
    setTimeout(() => {
      Cache.update(`shadowed_${key}`, (context, canvas, p) => {
        context.save();
        context.shadowColor = color(1, p);
        context.shadowBlur = 12;
        context.shadowOffsetX = 0;
        context.shadowOffsetY = 0;
        context.drawImage(canvas, 0, 0);
        // We draw it a second time for maximum glow
        context.drawImage(canvas, 0, 0);
        context.restore();
        context.globalCompositeOperation = 'destination-out';
        context.drawImage(canvas, 0, 0);
        context.globalCompositeOperation = 'source-over';
        context.drawImage(canvas, 0, 0);
      }, cached, playerIndex);
    }, 0);

    return highlighted;
  },
};

function skinify(id, playerIndex) {
  if (playerIndex >= 0) {
    if (Game.state.players[playerIndex].skins[id] !== undefined) {
      return Game.state.players[playerIndex].skins[id];
    }
  }
  return id;
}
