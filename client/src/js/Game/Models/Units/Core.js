/**
 * @file
 * Core unit
 *
 * TODO clean this up!
 */
import CoreRenderer from '../../../Graphics/Elements/Core';

export default function Core() {
  this.type = 'Core';
  this.player_id = 0;
  this.entity_id = 0;
  this.eid = 0;
  this.x = 0;
  this.y = 0;
  this.health_percentage = 0;
  this.state = 0;
  this.effects = [];

  // remove later
  this.stats = [];
}

Core.prototype.reset = function reset() {
  this.type = 'Core';
  this.player_id = 0;
  this.entity_id = 0;
  this.eid = 0;
  this.x = 0;
  this.y = 0;
  this.health_percentage = 0;
  this.state = 0;
  this.effects = [];

  // remove later
  this.stats = [];
}

Core.prototype.draw = function draw(context) {
  CoreRenderer.draw(context, this.x, this.y, this.player_id);
  CoreRenderer.healthBar(context, this.x, this.y, this.player_id, this.health_percentage);
};

Core.prototype.init = function init() {};
Core.prototype.drawMeta = function drawMeta() {};
