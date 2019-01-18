/**
 * @file
 * Renders the two-dimensional ProgressBox for level and stacks
 */
import Cache from '../Cache';
import { color } from '../Color';
import Event from '../../Event';

export default {
  draw(context, x, y, level, levelProgress, stacks, stackProgress, maxStacks) {
    // Get the proper background
    if (levelProgress !== false && stackProgress !== false) {
      context.drawImage(Cache.get('doubleProgressBox'), x, y);
    } else {
      context.drawImage(Cache.get('singleProgressBox'), x, y);
    }
    // Walls
    if (stacks !== false && level === false) {
      drawProgressBox(context, x + 56, y, false, stackProgress, stacks, stacks / maxStacks);
    }
    // Towers
    if (stacks === false && level !== false) {
      drawProgressBox(context, x + 56, y, true, levelProgress, level, level / 20);
    }
    // Minions
    if (stacks !== false && level !== false) {
      drawProgressBox(context, x, y, true, levelProgress, level, level / 20);
      drawProgressBox(context, x + 56, y, false, stackProgress, stacks, stacks / maxStacks);
    }
  },
};

function drawProgressBox(context, x, y, colored, progress, step, total) {
  // Fill color
  if (progress) {
    const h = 22 * progress;
    context.beginPath();
    context.rect(x + 15, y + (127 - h), 42, h);
    context.fillStyle = colored ? color(0.4) : 'rgba(200, 200, 200,0.4)';
    context.fill();
  }
  context.font = '22px "Exo 2"';
  context.strokeFillText(step, x + 36, y + 114, 'center', '#dbdbdb');
  // We always draw the bottom line
  context.beginPath();
  context.moveTo(x + 12, y + 130);
  context.lineTo(x + 60, y + 130);
  if (total) {
    // Don't overshoot!
    if (total > 1) total = 1;
    context.moveTo(x + 12, y + 130);
    context.lineTo(x + 12, y + (130 - (28 * total)));
    context.moveTo(x + 60, y + 130);
    context.lineTo(x + 60, y + (130 - (28 * total)));
    if (total === 1) {
      context.lineTo(x + 12, y + (130 - (28 * total)));
    }
  }
  context.lineWidth = 5;
  context.strokeStyle = '#000';
  context.stroke();
  context.lineWidth = 1;
  context.strokeStyle = colored ? color(1) : 'rgba(230, 230, 230,1)';
  context.stroke();
}

Event.on('cache clear', () => {
  Cache.set('singleProgressBox', 128, 132, (context) => {
    context.beginPath();
    context.rect(68, 100, 48, 32);
    context.fillStyle = '#0a0a0a';
    context.fill();
    context.lineWidth = 2;
    context.strokeStyle = '#000';
    context.stroke();
  });
  Cache.set('doubleProgressBox', 128, 132, (context) => {
    context.beginPath();
    context.rect(68, 100, 48, 32);
    context.fillStyle = '#0a0a0a';
    context.fill();
    context.lineWidth = 2;
    context.strokeStyle = '#000';
    context.stroke();
    context.beginPath();
    context.rect(12, 100, 48, 32);
    context.fillStyle = '#0a0a0a';
    context.fill();
    context.lineWidth = 2;
    context.strokeStyle = '#000';
    context.stroke();
  });
});
