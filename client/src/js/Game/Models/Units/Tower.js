/**
 * @file
 * Handles the tower entity
 */
import UnitCache from '../../../Graphics/UnitCache';
import { getLinear } from '../../../Graphics/Counters';
import UserInterface from '../../../Interface/UserInterface';
import Config from '../../../Config/Config';
import Cooldown from '../../../Graphics/Elements/Cooldown';

export default function Tower() {
  this.type = 'Tower';
  this.player_id = 0;
  this.entity_id = 0;
  this.eid = 0;
  this.x = 0;
  this.y = 0;
  this.vector_x = 0;
  this.vector_y = 0;
  this.angle = 0;
  this.attackRadius = null;
  this.attack_percentage = null;
  this.cooldown_percentage = 0;
  this.health_percentage = 0;
  this.state = 0;
  this.cooldown_seconds = 0;
  this.cooldown_percentage = 0;
  this.rotate = 0;
  this.effects = [];

  // remove later
  this.stats = [];
}

Tower.prototype.reset = function reset() {
  this.type = 'Tower';
  this.player_id = 0;
  this.entity_id = 0;
  this.eid = 0;
  this.x = 0;
  this.y = 0;
  this.vector_x = 0;
  this.vector_y = 0;
  this.angle = 0;
  this.attackRadius = null;
  this.attack_percentage = null;
  this.cooldown_percentage = 0;
  this.health_percentage = 0;
  this.state = 0;
  this.cooldown_seconds = 0;
  this.cooldown_percentage = 0;
  this.rotate = 0;
  this.effects = [];

  // remove later
  this.stats = [];
};

// @todo EXPAND THE INIT SYSTEM TO EVERY ENTITY
Tower.prototype.init = function init(data) {
  // Get the rotation parameter for this unit from the config
  const item = Config.query('towers', { id: this.eid });
  if (item.rotate !== undefined) {
    if (item.rotate === false) this.rotate = -1;
    if (item.rotate > 0) this.rotate = item.rotate;
  }

  // Add all parameters given to this object
  if (data) {
    Object.keys(data).forEach((prop) => {
      this[prop] = data[prop];
    });
  }
};

Tower.prototype.draw = function draw(context) {
  // Draw the unit to the canvas
  if (this.cooldown_percentage > 0) {
    Cooldown.draw(
      context,
      this.x,
      this.y,
      this.eid,
      this.player_id,
      'Tower',
      'interface',
      this.cooldown_percentage,
      this.cooldown_seconds,
    );
  } else {
    const sprite = UnitCache.get(this.eid, this.player_id, 'Tower');
    if (this.rotate === -1 || (this.rotate <= 0 && this.vector_x === 0 && this.vector_y === 0)) {
      // The default for targetless towers is UP!
      context.save();
      context.translate(this.x + 64, this.y + 64);
      context.rotate(-Math.PI / 2);
      context.drawImage(sprite, -64, -64);
      context.restore();
    } else if (this.rotate > 0 && !this.effects.includes(140977)) {
      // Rotate periodically
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
  }
}

Tower.prototype.drawMeta = function(context) {
  // Add a health bar
  context.healthBar(this.x, this.y, this.health_percentage);

  if (this.attack_percentage) {
    if (this.eid != 1104) { // Exception for the Radiator
      context.drawReload(this.x, this.y, this.attack_percentage);
    }
  }

  // Draw status effects
  if (this.effects.length) {
    context.statusFX(this.x, this.y, this.effects);
  }

  let factory = UserInterface.selectedFactory();
  if (factory) {
    if (factory.entity_id != undefined) {
      if (factory.entity_id == this.entity_id) {
        context.strokeStyle = 'rgba(255,255,255,0.75)';
        context.lineWidth = 1;
        context.beginPath();
        context.arc(this.x + 64, this.y + 64, this.attackRadius * 128, 0, 2 * Math.PI);
        context.closePath();
        context.stroke();
      }
    }
  }
}
