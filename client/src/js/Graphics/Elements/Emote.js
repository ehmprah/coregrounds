/**
 * @file
 * Displays user emotes
 */
import TWEEN from 'tween.js';
import Cache from '../Cache';
import Game from '../../Game/Game';
import Config from '../../Config/Config';
import Event from '../../Event';
import { color } from '../Color';

const activeEmotes = {};

Event.on('cache clear', () => {
  Cache.create('Emote_0', 1024, 128);
  Cache.create('Emote_1', 1024, 128);
});

export default {
  draw(context) {
    Object.keys(activeEmotes).forEach((emote) => {
      context.globalAlpha = activeEmotes[emote].alpha;
      context.drawImage(
        Cache.get(`Emote_${activeEmotes[emote].playerIndex}`),
        activeEmotes[emote].x,
        activeEmotes[emote].y
      );
      context.globalAlpha = 1;
      if (activeEmotes[emote].done === 1) {
        delete activeEmotes[emote];
      }
    });
  },

  show(playerIndex, emoteId) {
    let x;
    let y;

    if (Game.phase() === 1) {
      x = playerIndex === 1 ? 896 : 0;
      y = 448;
    } else {
      x = playerIndex === 1 ? 768 : 128;
      y = playerIndex === 1 ? 64 : 704;
    }

    activeEmotes[playerIndex] = {
      alpha: 0,
      playerIndex,
      x,
      y,
    };

    // Update the cache with the new emote
    Cache.update(`Emote_${playerIndex}`, cacheEmote, playerIndex, emoteId);

    // Start the fade in animation
    new TWEEN.Tween(activeEmotes[playerIndex])
      .to({ alpha: 0.9 }, 300)
      .easing(TWEEN.Easing.Quartic.In)
      .start();

    // And schedule the fade out animation
    new TWEEN.Tween(activeEmotes[playerIndex])
      .delay(4000)
      .to({ alpha: 0, done: 1 }, 600)
      .easing(TWEEN.Easing.Quartic.Out)
      .start();
  },
};

function cacheEmote(context, playerIndex, emoteId) {
  // We have to set the font first, or the text will not be measured correctly
  context.font = '36px "Exo 2"';
  context.fillStyle = color(0.3, playerIndex);
  context.clearRect(0, 0, 1024, 128);
  context.beginPath();
  // Set vars
  const emote = Config.query('emotes', { id: emoteId });
  const width = context.measureText(emote.emote).width + 128;
  // Add box depending on playerIndex and game phase
  if (playerIndex === 0) {
    // Add box from the left
    context.rect(32, 0, width, 128);
    // Add triangle
    context.moveTo(32, 128);
    context.lineTo(0, 128);
    context.lineTo(32, 96);
    context.lineTo(32, 128);
    context.fill();
    context.strokeFillText(emote.emote, 32 + (width / 2), 64);
  } else {
    const x = 992 - width;
    // Add box from the right
    context.rect(x, 0, width, 128);
    // Add triangle
    context.moveTo(1024, 0);
    context.lineTo(992, 32);
    context.lineTo(992, 0);
    context.lineTo(1024, 0);
    context.fill();
    context.strokeFillText(emote.emote, x + (width / 2), 64);
  }
}
