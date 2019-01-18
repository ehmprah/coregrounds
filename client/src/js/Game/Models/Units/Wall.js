/**
 * @file
 * Wall entity
 *
 * @todo find a solution for the damn health bars on walls!
 * @todo don't show stats for walls with towers
 */
import UnitCache from '../../../Graphics/UnitCache';

export default function Wall() {
  this.type = 'Wall';
  this.player_id = 0;
  this.entity_id = 0;
  this.eid = 0;
  this.x = 0;
  this.y = 0;
  this.health_percentage = 0;
  this.chainBonus = 0;
  this.effects = [];

  // remove later
  this.stats = [];
}

Wall.prototype.reset = function reset() {
  this.type = 'Wall';
  this.player_id = 0;
  this.entity_id = 0;
  this.eid = 0;
  this.x = 0;
  this.y = 0;
  this.health_percentage = 0;
  this.chainBonus = 0;
  this.effects = [];

  // remove later
  this.stats = [];
};

Wall.prototype.init = function init() {};

Wall.prototype.draw = function draw(context) {
  context.drawImage(UnitCache.get(1, this.player_id, 'Wall'), this.x, this.y);
};

Wall.prototype.drawMeta = function drawMeta(context) {
  context.healthBar(this.x, this.y, this.health_percentage);

  if (this.chainBonus) {
    context.chainBonus(this.x, this.y, this.chainBonus);
  }

  // Draw status effects
  if (this.effects.length) {
    context.statusFX(this.x, this.y, this.effects);
  }
};
