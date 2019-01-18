/**
 * @file
 * Trigger
 */
import Cache from '../../../Graphics/Cache';
import { color } from '../../../Graphics/Color';

export default function Trigger() {
  this.type = 'Trigger';
  this.player_id = 0;
  this.entity_id = 0;
  this.id = 0;
  this.x = 0;
  this.y = 0;
  this.health_percentage = 0;
  this.state = 0;

  // remove later
  this.stats = [];
}

Trigger.prototype.reset = function reset() {
  this.type = 'Trigger';
  this.player_id = 0;
  this.entity_id = 0;
  this.id = 0;
  this.x = 0;
  this.y = 0;
  this.health_percentage = 0;
  this.state = 0;

  // remove later
  this.stats = [];
};

Trigger.prototype.draw = function draw(ctx) {
  // Get/set unit cache
  let sprite = Cache.get(`unit_${this.id}_${this.player_id}`);
  if (sprite === false) {
    sprite = Cache.set(`unit_${this.id}_${this.player_id}`, 128, 128, (context, el, pl) => {
      context.globalAlpha = 0.5;
      context.unitSprite(el, color(1, pl), 128);
    }, this.id, this.player_id);
  }

  ctx.drawImage(sprite, this.x, this.y);
};

Trigger.prototype.init = function init() {};
Trigger.prototype.drawMeta = function drawMeta() {};
