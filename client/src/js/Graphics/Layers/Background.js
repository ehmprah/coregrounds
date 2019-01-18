/**
 * @file
 * The ingame background
 */
import Event from '../../Event';
import Cache from '../Cache';
import Game from '../../Game/Game';
import { color } from '../Color';

let canvas;
let context;

Event.on('cache clear', () => {
  canvas = Cache.create('BackgroundLayer', 1920, 1080);
  context = canvas.getContext('2d');
});

const Background = {

  draw(ctx) {
    ctx.drawImage(canvas, 0, 0);
  },

  cache() {
    context.lineWidth = 0.5;
    context.beginPath();
    for (let y = 0; y < 896; y += 128) {
      for (let x = 0; x < 1920; x += 128) {
        context.rect(x + 4, y + 4, 120, 120);
      }
    }
    // Add gradient with the player colors
    const gradient = context.createLinearGradient(0, 1080, 1920, 0);
    if (Game.playerIndex === 0) {
      gradient.addColorStop(0, color(0.7, 0));
      gradient.addColorStop(1, color(0.7, 1));
    } else {
      gradient.addColorStop(0, color(0.7, 1));
      gradient.addColorStop(1, color(0.7, 0));
    }
    context.strokeStyle = gradient;
    context.stroke();
  },
};

Event.on('match colors ready', () => {
  Background.cache();
});

export default Background;
