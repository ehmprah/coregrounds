/**
 * @file
 * The main render loop
 */
import TWEEN from 'tween.js';
import Event from '../Event';
import Game from '../Game/Game';
// import Backgrounds from './Backgrounds';
import Particles from './Particles/Particles';
import Background from './Layers/Background';
import Midground from './Layers/Midground';
import Foreground from './Layers/Foreground';

// Include the extensions for the canvas context
import './Context';
// Add sprite handling to the canvas context
import './Sprites';
// Include and start the global counters
import './Counters';

let context;
let last = 0;
let delta;

let beginTime = performance.now();
let prevTime = beginTime;
let frames = 0;

let shake = 0;
let lastShake = 0;

const Graphics = {
  fps: 0,
  screenshake() {
    if (lastShake < Date.now() - 15000) {
      shake = 10;
      lastShake = Date.now();
    }
  },
};

export default Graphics;

Event.on('app loaded', () => {
  // Set up the canvas context
  const map = document.getElementById('coregrounds');
  map.width = 1920;
  map.height = 1080;
  context = map.getContext('2d');
  // Start the animation loop
  requestAnimationFrame(animate);
});

function animate(time) {
  // Request the next step in the loop
  requestAnimationFrame(animate);

  // Start measuring FPS
  beginTime = performance.now();

  // Calculate delta time in seconds
  delta = (time - last) / 1000;
  last = time;

  // Update animations
  TWEEN.update();

  // Clear screen always, because of the steam overlay!
  context.clearRect(0, 0, 1920, 1080);

  if (shake) {
    context.save();
    context.translate(Math.random() * 5, Math.random() * 5);
  }

  // Draw game stuff
  if (Game.status >= 2) {
    Background.draw(context);
    Midground.draw(context);
  }
  Particles.update(delta);
  Foreground.update();
  Foreground.draw(context);

  if (shake) {
    context.restore();
    shake -= 1;
  }

  // Finish FPS calculation
  frames += 1;

  if (time > prevTime + 1000) {
    Graphics.fps = Math.round((frames * 1000) / (time - prevTime));
    prevTime = time;
    frames = 0;
  }
}
