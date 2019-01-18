/**
 * @file Overlays.js
 * Handles different overlays and their display
 *
 * TODO add slightly pulsating effect (if we animate, update the intermediate cache!)
 */
import TWEEN from 'tween.js';
import Cache from '../Cache';
import MoveOverlay from './MoveOverlay';
import './SpawnOverlay';

let visible = false;
const animation = {
  alpha: 0,
};
let hide;

export default {

  visible() {
    return visible;
  },

  show(type, factory = false) {
    if (type === 'MoveOverlay' && factory) {
      MoveOverlay.setFactory(factory);
    }

    let delay = 0;

    if (hide && hide.update) {
      hide.stop();
    }

    if (visible !== false) {
      this.hide();
      delay = 100;
    }

    new TWEEN.Tween(animation)
      .delay(delay)
      .to({ alpha: 1 }, 100)
      .easing(TWEEN.Easing.Quartic.In)
      .onStart(() => {
        visible = type;
      })
      .start();
  },

  hide() {
    hide = new TWEEN.Tween(animation)
      .to({ alpha: 0 }, 100)
      .easing(TWEEN.Easing.Quartic.Out)
      .onComplete(() => {
        visible = false;
      })
      .start();
  },

  draw(ctx) {
    if (visible !== false) {
      ctx.globalAlpha = animation.alpha;
      ctx.drawImage(Cache.get(visible), 0, 0);
      ctx.globalAlpha = 1;
    }
  },

};
