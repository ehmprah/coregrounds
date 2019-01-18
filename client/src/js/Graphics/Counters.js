/**
 * @file
 * Keeps a global linear and yoyo counter regardless of the game state.
 */
import TWEEN from 'tween.js';
import Event from '../Event';

const counters = {
  linear: 0,
  yoyo: 0,
};

Event.on('cache clear', () => {
  // We need a slow, client side loop regardless of networking and rendering
  setInterval(() => {
    Event.fire('every second');
  }, 1000);

  // Start the linear counter
  new TWEEN.Tween(counters)
    .to({ linear: '+1000' }, 1000)
    .easing(TWEEN.Easing.Linear.None)
    .repeat(Infinity)
    .start();

  // Start the yoyo counter
  new TWEEN.Tween(counters)
    .to({ yoyo: 1000 }, 1000)
    .delay(1000)
    .yoyo(true)
    .easing(TWEEN.Easing.Linear.None)
    .repeat(Infinity)
    .start();
});

export function getLinear() {
  return counters.linear;
}

export function getYoyo() {
  return counters.yoyo;
}
