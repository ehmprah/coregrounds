/**
 * @file
 * Displays tooltips with a unit's description
 */
import TWEEN from 'tween.js';
import Cache from '../Cache';
import Event from '../../Event';
import { color } from '../Color';
import UnitCache from '../UnitCache';

// Set private variables
let timeout;
let tween;
const tooltip = {
  alpha: 0,
  id: 0,
};

Event.on('cache clear', () => {
  Cache.create('UnitTooltip', 1920, 256);
});

// Public functions show, hide, draw
export default {

  draw(context) {
    if (tooltip.id > 0) {
      // Set alpha accordingly
      context.globalAlpha = tooltip.alpha;
      // Draw the tooltip
      context.drawImage(Cache.get('UnitTooltip'), 0, 384);
      // Reset alpha
      context.globalAlpha = 1;
    }
  },

  show(pick) {
    // Don't try to show this anew
    if (tooltip.id > 0) return;
    if (pick.status === 'picked' || pick.status === 'banned') {
      // Debounce tooltip animation
      if (timeout) {
        clearTimeout(timeout);
        TWEEN.remove(tween);
        tooltip.alpha = 0;
      }
      // Create a new tooltip in 100ms
      timeout = setTimeout(() => {
        // Update tooltip info
        tooltip.id = pick.id;
        // Update the cache
        updateCache(pick);
        // Start animation
        tween = new TWEEN.Tween(tooltip)
          .to({ alpha: 0.95 }, 200)
          .easing(TWEEN.Easing.Quartic.In)
          .start();
      }, 100);
    }
  },

  hide() {
    // Remove debounced tooltips
    if (timeout) {
      clearTimeout(timeout);
      TWEEN.remove(tween);
      tooltip.alpha = 0;
    }
    if (tooltip.id !== 0) {
      tooltip.id = 0;
      // Remove active animations
      TWEEN.remove(tween);
      // And add a new one hiding the tooltip
      tween = new TWEEN.Tween(tooltip)
        .to({ alpha: 0 }, 200)
        .easing(TWEEN.Easing.Quartic.Out)
        .start();
    }
  },
};

function updateCache(passed) {
  Cache.update('UnitTooltip', (context, pick) => {
    context.clearRect(0, 0, 1920, 256);
    context.fillStyle = '#000';
    context.fillRect(0, 16, 1920, 224);
    const cached = UnitCache.get(pick.id, pick.playerIndex, pick.type, 'interface');
    context.drawImage(cached, 384, 48);
    context.font = '36px "Exo 2"';
    context.strokeFillText(pick.type.toUpperCase(), 544, 76, 'left');
    context.strokeFillText(pick.name.toUpperCase(), 682, 76, 'left', color(1, pick.playerIndex));
    context.font = '28px "Exo 2"';
    context.strokeFillText(pick.description, 544, 128, 'left');
    context.strokeFillText(`Difficulty: ${pick.difficulty} `, 544, 180, 'left', '#777');
    context.strokeFillText(`Role: ${pick.role} `, 800, 180, 'left', '#777');
    // Display strong and weak picks
    context.strokeFillText('Strong vs:', 1020, 180, 'left', '#777');
    pick.strong.forEach((counter, index) => {
      const sprite = UnitCache.get(counter.id, (1 - pick.playerIndex), counter.type, 'interface');
      setTimeout(() => {
        context.drawImage(sprite, 1150 + (index * 48), 158, 48, 48);
      }, 0);
    });
    context.strokeFillText('Weak vs:', 1320, 180, 'left', '#777');
    pick.weak.forEach((counter, index) => {
      const sprite = UnitCache.get(counter.id, (1 - pick.playerIndex), counter.type, 'interface');
      setTimeout(() => {
        context.drawImage(sprite, 1450 + (index * 48), 158, 48, 48);
      }, 0);
    });
  }, passed);
}
