/**
 * @file
 * Displays the spawn overlay
 */
import { color } from '../Color';
import Cache from '../Cache';
import Event from '../../Event';

let canvas;
let context;

Event.on('cache clear', () => {
  canvas = Cache.create('SpawnOverlay', 1920, 896);
  context = canvas.getContext('2d');
});

Event.on('game started', () => {
  context.clearRect(0, 0, 1920, 896);
  context.lineWidth = 3;

  const gradient = context.createLinearGradient(0, 0, 0, 888);
  gradient.addColorStop(0, color(1));
  gradient.addColorStop(1, color(0));
  context.strokeStyle = gradient;

  context.strokeRect(4, 4, 1912, 888);
  context.font = '24px "Exo 2"';
  context.fillStyle = color();
  context.fillText('SPAWN MODE', 20, 40);
});
