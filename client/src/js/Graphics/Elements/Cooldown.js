/**
 * @file
 * Takes image data and displays a grayscale clipped version of it
 */
import Cache from '../Cache';
import UnitCache from '../UnitCache';

export default {
  draw(context, x, y, id, playerIndex, type, prefix, progress, seconds) {
    // Get cached version
    const cached = UnitCache.get(id, playerIndex, type, prefix);
    // Draw the normal version
    context.drawImage(cached, x, y);
    // If we have a cooldown, overlay this with the darkened version
    if (seconds > 0) {
      // Get or create a darkened version of the sprite
      let bw = Cache.get(`cooldown_${id}_${playerIndex}`);
      if (!bw) {
        bw = Cache.create(`cooldown_${id}_${playerIndex}`, 128, 128);
        // Draw the sprite asynchronously to avoid empty caches
        setTimeout(() => {
          const ctx = bw.getContext('2d');
          // Draw the normal version
          ctx.drawImage(cached, 0, 0);
          // Only paint on overlap with current shape
          ctx.globalCompositeOperation = 'source-in';
          // Massively darken the sprite
          ctx.fillStyle = 'rgba(0, 0, 0, 0.75)';
          ctx.fillRect(0, 0, 128, 128);
        }, 0);
      }
      const angle = 2 * progress * Math.PI;
      // Clip the area
      context.save();
      context.moveTo(x + 64, y + 64);
      context.arc(x + 64, y + 64, 64, (1.5 * Math.PI) + angle, 3.5 * Math.PI, false);
      context.clip();
      // Draw the dark version
      context.drawImage(bw, x, y);
      // Restore old context
      context.restore();
      // Draw remaining seconds
      context.drawSeconds(x, y, seconds);
    }
  },
};
