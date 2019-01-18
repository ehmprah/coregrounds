/**
 * @file
 * Minion unit
 */
import UnitCache from '../../../Graphics/UnitCache';
import { getLinear } from '../../../Graphics/Counters';
import Config from '../../../Config/Config';
import Game from '../../../Game/Game';

export default function Minion() {
  this.type = 'Minion';
  this.player_id = 0;
  this.entity_id = 0;
  this.eid = 0;
  this.x = 0;
  this.y = 0;
  this.vector_x = 0;
  this.vector_y = 0;
  this.angle = 0;
  this.attack_percentage = null;
  this.health_percentage = 0;
  this.foremost = false;
  this.state = 0;
  this.rotate = 0;
  this.effects = [];

  // remove later
  this.stats = [];
}

Minion.prototype.reset = function reset() {
  this.type = 'Minion';
  this.player_id = 0;
  this.entity_id = 0;
  this.eid = 0;
  this.x = 0;
  this.y = 0;
  this.vector_x = 0;
  this.vector_y = 0;
  this.angle = 0;
  this.attack_percentage = null;
  this.health_percentage = 0;
  this.foremost = false;
  this.state = 0;
  this.rotate = 0;
  this.effects = [];

  // remove later
  this.stats = [];
};

Minion.prototype.init = function init() {
  // Get the rotation parameter for this unit from the config
  const item = Config.query('minions', { id: this.eid });
  if (item.rotate !== undefined) {
    if (item.rotate === false) this.rotate = -1;
    if (item.rotate > 0) this.rotate = item.rotate;
  }
};

Minion.prototype.draw = function draw(context) {
  const sprite = this.foremost && Game.state.players[this.player_id].minion_limit === 1
    ? UnitCache.get(this.eid, this.player_id, 'Minion', 'foremost')
    : UnitCache.get(this.eid, this.player_id, 'Minion');

  // Draw the unit to the canvas
  if (this.rotate === -1 || (this.rotate <= 0 && this.angle === 0)) {
    // Normal draw
    context.drawImage(sprite, this.x, this.y);
  } else if (this.rotate > 0 && !this.effects.includes(140977)) {
    const rotation = 1000 / this.rotate;
    context.save();
    context.translate(this.x + 64, this.y + 64);
    context.rotate(((getLinear() % rotation) / rotation) * (2 * Math.PI));
    context.drawImage(sprite, -64, -64);
    context.restore();
  } else {
    context.save();
    context.translate(this.x + 64, this.y + 64);
    context.rotate(this.angle);
    context.drawImage(sprite, -64, -64);
    context.restore();
  }
};

Minion.prototype.drawMeta = function drawMeta(context) {
  context.healthBar(this.x, this.y, this.health_percentage);

  if (this.attack_percentage) {
    // Exception for Sawblade, Container, Rocket & Bullet
    if (this.eid !== 1011 && this.eid !== 1014 && this.eid !== 1001 && this.eid !== 101001) {
      context.drawReload(this.x, this.y, this.attack_percentage);
    }
  }

  // Draw status effects
  if (this.effects.length) {
    context.statusFX(this.x, this.y, this.effects);
  }
};
