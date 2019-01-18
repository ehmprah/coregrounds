/**
 * @file Pick.js
 * A single pick object
 *
 * @todo REWRITE TO ES6
 * @todo check dependencies
 */
import TWEEN from 'tween.js';
import Box from '../../Game/Models/Box';
import Game from '../../Game/Game';
import Picks from './Picks';
import UnitCache from '../UnitCache';
import UnitTooltip from './UnitTooltip';
import { isOnScreen } from '../../Util/Geometry';
import Config from '../../Config/Config';

export default function Pick(config) {
  this.status = 'off';
  this.selected = false;
  this.playerIndex = Game.playerIndex;

  // We keep several values on this object for easier retrieval
  this.id = config.id;
  this.name = config.name;
  this.description = config.description;
  this.role = config.role;
  this.difficulty = config.difficulty;
  this.strong = [];
  config.strong.forEach((id) => {
    this.strong.push({ id, type: Config.getUnitTypeFromId(id) });
  });
  this.weak = [];
  config.weak.forEach((id) => {
    this.weak.push({ id, type: Config.getUnitTypeFromId(id) });
  });

  // Determine the type based on the config item
  this.type = 'Ability';
  if (config.tower !== undefined) this.type = 'Tower';
  if (config.minion !== undefined) this.type = 'Minion';

  // Add box for clicks and hover effects
  this.box = new Box(-128, 1208, 128, 128, true);
  // @todo check if we need this. We don't stop those, do we?
  this.tween = new TWEEN.Tween(this.box);

  // Add handlers for unit tooltips
  this.box.on('mousein', UnitTooltip.show, this);
  this.box.on('mouseout', UnitTooltip.hide);
  // TODO maybe add touchstart and touchend as well?

  // Add mousedown handler
  this.box.on('mousedown', (pick) => {
    if (pick.status === 'pickable') {
      Picks.select(pick.id);
    }
  }, this);
}

Pick.prototype.isPickable = function pickable() {
  return this.status === 'pickable';
};

Pick.prototype.select = function select() {
  this.selected = true;
};

Pick.prototype.deselect = function deselect() {
  this.selected = false;
};

Pick.prototype.setPickable = function setPickable() {
  this.status = 'pickable';
  this.box.listening = true;
};

Pick.prototype.pickable = function pickable() {
  return this.status !== 'picked' && this.status !== 'banned';
};

Pick.prototype.isNotPickedOrBanned = function isNotPickedOrBanned() {
  return this.status !== 'picked' && this.status !== 'banned';
};

Pick.prototype.reset = function reset() {
  this.box.listening = false;
  this.status = 'off';
};

Pick.prototype.setOff = function setOff() {
  this.box.listening = false;
  this.status = 'off';
  this.tween.to({ y: 1208 }, 500).easing(TWEEN.Easing.Quartic.Out).start();
};

Pick.prototype.updateStatus = function updateStatus(status, playerIndex, i) {
  if (this.status === status) return;
  this.status = status;
  this.playerIndex = playerIndex;
  this.selected = false;
  this.box.listening = true;
  if (this.type === 'Ability') {
    if (status === 'banned') {
      this.box.x = 64 + (playerIndex * 1664);
    } else {
      this.box.x = 384 + (i * 128) + (playerIndex * 768);
    }
    this.box.y = -128;
    this.tween.to({ y: 256 }, 300).easing(TWEEN.Easing.Bounce.Out).start();
  } else {
    if (status === 'banned') {
      this.box.x = 64 + (playerIndex * 1664);
    } else {
      this.box.x = 256 + (i * 128) + (playerIndex * 768);
    }
    this.box.y = -128;
    this.tween.to({ y: 128 }, 300).easing(TWEEN.Easing.Bounce.Out).start();
  }
};

Pick.prototype.draw = function draw(context) {
  if (!isOnScreen(this.box)) {
    return;
  }

  let { y } = this.box;
  if (this.box.active && this.status === 'pickable') {
    y += 2;
  }

  let sprite;
  if (this.status === 'banned' || Game.state.draft <= 2) {
    sprite = this.selected
      ? UnitCache.getHighlight(this.id, -1, this.type, 'interface')
      : UnitCache.get(this.id, -1, this.type, 'interface');
  } else {
    sprite = this.selected
      ? UnitCache.getHighlight(this.id, this.playerIndex, this.type, 'interface')
      : UnitCache.get(this.id, this.playerIndex, this.type, 'interface');
  }

  // Draw the sprite
  if (this.status === 'pickable' && Game.activePlayerIndex !== Game.playerIndex) {
    context.globalAlpha = 0.5;
  }
  context.drawImage(sprite, this.box.x, y);
  context.globalAlpha = 1;
};
