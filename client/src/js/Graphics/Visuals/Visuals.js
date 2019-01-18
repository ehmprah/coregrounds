/**
 * @file
 * A drop animation for units
 *
 * TODO rewrite to ES6
 * TODO remove type clutter, move sounds to above for fewer methods
 */
import TWEEN from 'tween.js';
import Config from '../../Config/Config';
import Input from '../../Input';
import Sound from '../../Sound';
import VisualPool from './VisualPool';
import { jitter } from '../../Util';
import { color } from '../Color';

const visuals = [];

export default {
  draw(context) {
    if (visuals.length) {
      visuals.forEach((visual, index) => {
        // Garbage collect
        if (visual.done >= 1) {
          // Return to pool
          VisualPool.free(visual);
          // And delete from the active visuals
          visuals.splice(index, 1);
        // Or draw the effect
        } else {
          visual.draw(context);
        }
      });
    }
  },

  show(id, playerIndex, x, y) {
    // Mass production
    if (id === 1410) {
      // TODO maybe add a little animation here?
      Sound.play('buff');
    }

    // Demolish, Wallswap, Dynablaster
    if (id === 1402 || id === 1414 || id === 1416) {
      abilityIcon(id, playerIndex, x, y, 1000);
    }

    // Teleport
    if (id === 1412) {
      abilityIcon(id, playerIndex, x, y, 1000, 10);
    }

    // IonStrike Icon
    if (id === 1403) {
      Sound.play('ionstrike_onset');
      abilityIcon(id, playerIndex, x, y, 5000);
    }

    // IonStrike Blast
    if (id === 140301) {
      Sound.play('ionstrike_strike');
      ionStrike(playerIndex, x, y);
    }

    // Dynablaster
    if (id === 141601) {
      Sound.play('ionstrike_strike');
      dynablast(playerIndex, x, y);
    }

    // Ability: Adrenaline
    if (id === 1411) {
      // Get range from config
      const spell = Config.query('spells', { id: 14114499 });
      // Create the visual
      area(x, y, (spell.area.circle.radius * 128), color(1, playerIndex), true, 1411, playerIndex);
      // Play sound after the windup is complete
      Sound.play('buff', Config.DURATION_AOE_WINDUP);
    }

    // Ability: Lifelink
    if (id === 1413) {
      // Get range from config
      const spell = Config.query('spells', { id: 14134499 });
      // Create the visual
      area(x, y, (spell.area.circle.radius * 128), color(1, playerIndex), true, 1413, playerIndex);
      // Play sound after the windup is complete
      Sound.play('buff', Config.DURATION_AOE_WINDUP);
    }

    // Ability: Heal
    if (id === 1400) {
      // Get range from config
      const spell = Config.query('spells', { id: 14004499 });
      // Create the visual
      area(x, y, (spell.area.circle.radius * 128), Config.COLOR_HEALING, true, 1400, playerIndex);
      // Play sound after the windup is complete
      Sound.play('heal', Config.DURATION_AOE_WINDUP);
    }

    // Ability: Explode
    if (id === 1401) {
      // Get range from config
      const spell = Config.query('spells', { id: 14014499 });
      // Create the visual
      area(x, y, (spell.area.circle.radius * 128), color(1, playerIndex), true, 1401, playerIndex);
      // Play sound after the windup is complete
      Sound.play('shot_aoe', Config.DURATION_AOE_WINDUP);
    }

    // Ability: Hurry
    if (id === 1405) {
      // Get range from config
      const spell = Config.query('spells', { id: 14054499 });
      // Create the visual
      area(x, y, (spell.area.circle.radius * 128), color(1, playerIndex), true, 1405, playerIndex);
      // Play sound after the windup is complete
      Sound.play('debuff', Config.DURATION_AOE_WINDUP);
    }

    // Ability: Slow
    if (id === 1406) {
      // Get range from config
      const spell = Config.query('spells', { id: 14064499 });
      // Create the visual
      area(x, y, (spell.area.circle.radius * 128), color(1, playerIndex), true, 1406, playerIndex);
      // Play sound after the windup is complete
      Sound.play('buff', Config.DURATION_AOE_WINDUP);
    }

    // Ability: Frenzy
    if (id === 1407) {
      // Get range from config
      const spell = Config.query('spells', { id: 14074499 });
      // Create the visual
      area(x, y, (spell.area.circle.radius * 128), color(1, playerIndex), true, 1407, playerIndex);
      // Play sound after the windup is complete
      Sound.play('buff', Config.DURATION_AOE_WINDUP);
    }

    // Ability: Shield
    if (id === 1408) {
      // Get range from config
      const spell = Config.query('spells', { id: 14084499 });
      // Create the visual
      area(x, y, (spell.area.circle.radius * 128), color(1, playerIndex), true, 1408, playerIndex);
      // Play sound after the windup is complete
      Sound.play('buff', Config.DURATION_AOE_WINDUP);
    }

    // Ability: Stun
    if (id === 1409) {
      // Get range from config
      const spell = Config.query('spells', { id: 14094499 });
      // Create the visual
      area(x, y, (spell.area.circle.radius * 128), color(1, playerIndex), true, 1409, playerIndex);
      // Play sound after the windup is complete
      Sound.play('debuff', Config.DURATION_AOE_WINDUP);
    }

    // Ability: Exhaust
    if (id === 1415) {
      // Get range from config
      const spell = Config.query('spells', { id: 14154499 });
      // Create the visual
      area(x, y, (spell.area.circle.radius * 128), color(1, playerIndex), true, 1415, playerIndex);
      // Play sound after the windup is complete
      Sound.play('buff', Config.DURATION_AOE_WINDUP);
    }

    // Ability: Slow
    if (id === 1417) {
      // Get range from config
      const spell = Config.query('spells', { id: 14174499 });
      // Create the visual
      area(x, y, (spell.area.circle.radius * 128), color(1, playerIndex), true, 1417, playerIndex);
      // Play sound after the windup is complete
      Sound.play('debuff', Config.DURATION_AOE_WINDUP);
    }

    // Minion Limit
    if (id === 999001) {
      // Get range from config
      const spell = Config.query('spells', { id: 99945 }); // action vs. trigger
      // Create the visual
      area(x, y, (spell.area.circle.radius * 128), color(1, playerIndex), true, 0, playerIndex);
      // Play sound after the windup is complete
      Sound.play('shot_aoe', Config.DURATION_AOE_WINDUP);
    }

    // Artillery explosion
    if (id === 1111) { // TODO update to new range handling!
      // Create the visual
      area(x, y, 128, color(1, playerIndex), false);
      // Play sound after the windup is complete
      Sound.play('shot_aoe');
    }

    // Shield Generator "projectile"
    if (id === 1013) { // TODO update to new range handling!
      // Get range from config
      const unit = Config.query('minions', { id: 1013 });
      // Create the visual
      area(x, y, (unit.stats.attack_range * 128), color(1, playerIndex), false);
      // Play sound after the windup is complete
      Sound.play('shot_aoe');
    }

    // Freezer "projectile"
    if (id === 1102) { // TODO update to new range handling!
      // Get range from config
      const unit = Config.query('towers', { id: 1102 });
      // Create the visual
      area(x, y, (unit.stats.attack_range * 128), color(1, playerIndex), false);
      // Play sound after the windup is complete
      Sound.play('debuff');
    }

    // Boomblaster "projectile"
    if (id === 1108) { // TODO update to new range handling!
      // Get range from config
      const unit = Config.query('towers', { id: 1108 });
      // Create the visual
      area(x, y, (unit.stats.attack_range * 128), color(1, playerIndex), false);
      // Play sound after the windup is complete
      Sound.play('shot_aoe');
    }

    // Healer "projectile"
    if (id === 1109) { // TODO update to new range handling!
      // Get range from config
      const unit = Config.query('towers', { id: 1109 });
      // Create the visual
      area(x, y, (unit.stats.attack_range * 128), Config.COLOR_HEALING, false);
      // Play sound after the windup is complete
      Sound.play('heal');
    }
  },

  unitDrop(id, cursor) {
    // Get the current cursor position for placement
    if (!cursor) cursor = Input.position();
    // Get a new particle from the pool
    const visual = VisualPool.get();
    visual.id = id;
    visual.type = 'unitDrop';
    visual.x = cursor.x - 128;
    visual.y = cursor.y - 128;
    visual.a = 2;
    // Animate it
    new TWEEN.Tween(visual)
      .to({
        x: 0,
        y: 768,
        a: 1,
        done: 1,
      }, 400)
      .easing(TWEEN.Easing.Cubic.In).start();
    // Finally add the particle to our array
    visuals.push(visual);
  },

  wallDrop(playerIndex) {
    // Get the current cursor position for placement
    const cursor = Input.position();
    // Get a new particle from the pool
    const visual = VisualPool.get();
    visual.playerIndex = playerIndex;
    visual.type = 'wallDrop';
    visual.x = cursor.x - 128;
    visual.y = cursor.y - 128;
    visual.a = 2;
    // Animate it
    new TWEEN.Tween(visual)
      .to({
        x: visual.x + 64,
        y: visual.y + 64,
        a: 1,
        done: 1,
      }, 400)
      .easing(TWEEN.Easing.Cubic.In).start();
    // Finally add the particle to our array
    visuals.push(visual);
  },

};

function ionStrike(playerIndex, x, y) {
  let visual;
  // Animate with 5 jittered beams for maximum effect
  for (let n = 0; n < 5; n++) {
    // Get a new particle from the pool
    visual = VisualPool.get();
    visual.playerIndex = playerIndex;
    visual.type = 'ionStrike';
    visual.x = x + jitter(12);
    visual.y = y + jitter(12);
    visual.a = 12;
    // Animate it
    new TWEEN.Tween(visual).to({ a: 24, done: 1 }, 500).start();
    // Finally add the particle to our array
    visuals.push(visual);
  }
}

function dynablast(playerIndex, x, y) {
  let visual;
  ['H', 'V'].forEach((direction) => {
    // Animate with 5 jittered beams for maximum effect
    for (let n = 0; n < 5; n++) {
      // Get a new particle from the pool
      visual = VisualPool.get();
      visual.playerIndex = playerIndex;
      visual.type = `dynablast${direction}`;
      visual.x = x + jitter(12);
      visual.y = y + jitter(12);
      visual.a = 12;
      // Animate it
      new TWEEN.Tween(visual).to({ a: 24, done: 1 }, 500).start();
      // Finally add the particle to our array
      visuals.push(visual);
    }
  });
}

function abilityIcon(id, playerIndex, x, y, duration, rotate = 0) {
  // Get a new particle from the pool
  const visual = VisualPool.get();
  visual.id = id;
  visual.playerIndex = playerIndex;
  visual.type = 'abilityIcon';
  visual.x = x - 64;
  visual.y = y - 64;
  visual.a = rotate;
  // Animate it
  new TWEEN.Tween(visual).to({ done: 1 }, duration).start();
  // Finally add the particle to our array
  visuals.push(visual);
}

function area(x, y, range, clr, delay = true, icon = 0, playerIndex = 0) {
  // Get a new particle from the pool
  const visual = VisualPool.get();
  visual.type = 'area';
  visual.playerIndex = playerIndex;
  visual.x = x;
  visual.y = y;
  // Current range
  visual.a = range;
  // Set color
  visual.c = clr;
  // Set icon
  visual.id = icon;
  if (delay) {
    visual.b = range;
    // We begin at 0 range
    visual.a = 0;
    new TWEEN.Tween(visual)
      .delay(1000)
      .to({ a: range, done: 1 }, 250)
      .easing(TWEEN.Easing.Cubic.In)
      .start();
  } else {
    // We begin at 0 range
    visual.a = 0;
    new TWEEN.Tween(visual).to({ a: range, done: 1 }, 250).start();
  }
  // Finally add the particle to our array
  visuals.push(visual);
}
