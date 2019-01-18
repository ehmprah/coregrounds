/**
 * @file
 * Handles the projectile entity
 */
import UnitCache from '../../../Graphics/UnitCache';
import { color } from '../../../Graphics/Color';
import { getLinear } from '../../../Graphics/Counters';
import Sound from '../../../Sound';
import Config from '../../../Config/Config';
import { randomNumber, randomSignedInt, jitter } from '../../../Util';

export default function Projectile() {
  this.id = 0;
  this.type = 'Projectile';
  this.player_id = 0;
  this.entity_id = 0;
  this.x = 0;
  this.y = 0;
  this.start_x = 0;
  this.start_y = 0;
  this.target_x = 0;
  this.target_y = 0;
  this.vector_x = 0;
  this.vector_y = 0;
  this.angle = 0;
  this.health_percentage = 0;
  this.state = 0;
  this.rotate = 0;
  this.effects = [];

  // remove later
  this.stats = [];
}

Projectile.prototype.reset = function reset() {
  this.id = 0;
  this.type = 'Projectile';
  this.player_id = 0;
  this.entity_id = 0;
  this.x = 0;
  this.y = 0;
  this.start_x = 0;
  this.start_y = 0;
  this.target_x = 0;
  this.target_y = 0;
  this.vector_x = 0;
  this.vector_y = 0;
  this.angle = 0;
  this.health_percentage = 0;
  this.state = 0;
  this.rotate = 0;
  this.effects = [];

  // remove later
  this.stats = [];
};

Projectile.prototype.init = function init() {
  // Get the rotation parameter for this unit from the config
  const item = Config.query('projectiles', { id: this.eid });
  if (item.rotate !== undefined) {
    if (item.rotate === false) this.rotate = -1;
    if (item.rotate > 0) this.rotate = item.rotate;
  }
};

Projectile.prototype.draw = function draw(context) {
  // The lightning doesn't have a sprite and is dynamically rendered
  if (this.id === 1207) {
    let fx;
    let fy;
    let tx;
    let ty;
    context.save();
    context.lineWidth = 2;
    context.strokeStyle = color(1, this.player_id);
    context.shadowColor = color(1, this.player_id);
    context.shadowBlur = 10;
    let t;
    let x;
    let y;
    // We create 3 streaks of lightning
    for (let n = 0; n < 3; n++) {
      fx = this.start_x + 64;
      fy = this.start_y + 64;
      tx = this.target_x + 64;
      ty = this.target_y + 64;
      context.beginPath();
      context.moveTo(fx, fy);
      t = 0;
      for (let i = 0; i <= 3; i++) {
        // Generate a random number between 1 and the last t
        t = randomNumber(1, t);
        // Find the corresponding point on the line
        x = ((1 - t) * fx) + (t * tx);
        y = ((1 - t) * fy) + (t * ty);
        // Jitter the point
        x += randomSignedInt(-20, 20);
        y += randomSignedInt(-20, 20);
        // Move to it
        context.lineTo(x, y);
      }
      // Move to the end point and stroke
      context.lineTo(tx, ty);
      context.stroke();
    }
    context.restore();
    return;
  }

  // As is the the sunbeam
  if (this.id === 1204) {
    context.save();
    context.lineWidth = 6;
    context.strokeStyle = color(1, this.player_id);
    context.shadowColor = color(1, this.player_id);
    context.shadowBlur = 10;
    context.beginPath();
    context.moveTo(
      this.start_x + 64 + jitter(2),
      this.start_y + 64 + jitter(2)
    );
    context.lineTo(
      this.target_x + 64 + jitter(2),
      this.target_y + 64 + jitter(2)
    );
    context.stroke();
    context.restore();
    return;
  }

  // As is the the healray
  if (this.id === 1215) {
    context.save();
    context.lineWidth = 6;
    context.strokeStyle = Config.COLOR_HEALING;
    context.shadowColor = Config.COLOR_HEALING;
    context.shadowBlur = 10;
    context.beginPath();
    context.moveTo(
      this.start_x + 64 + jitter(2),
      this.start_y + 64 + jitter(2)
    );
    context.lineTo(
      this.target_x + 64 + jitter(2),
      this.target_y + 64 + jitter(2)
    );
    context.stroke();
    context.restore();
    return;
  }

  // Get/set unit cache
  const sprite = UnitCache.get(this.id, this.player_id, 'Projectile');

  // Draw the unit to the canvas
  if (this.rotate === -1 || (this.rotate <= 0 && this.angle === 0)) {
    // Normal draw
    context.drawImage(sprite, this.x, this.y);
  } else if (this.rotate > 0) {
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

Projectile.prototype.drawMeta = function drawMeta() {};

Projectile.prototype.sound = function sound(first) {
  // Normal shot
  if (this.id === 1201) {
    Sound.play('shot_medium');
  }

  // Rapid
  if (this.id === 1202) {
    Sound.play('shot_fast');
  }

  // Heavy
  if (this.id === 1203) {
    Sound.play('shot_slow');
  }

  // Sunbeam
  if (this.id === 1204) {
    Sound.play('shot_railgun_light');
  }

  // Artillery
  if (this.id === 1206) {
    Sound.play('shot_medium');
  }

  // Shuriken & Chakram
  if (this.id === 1205 ||
      this.id === 1208 ||
      this.id === 1209 ||
      this.id === 1210 ||
      this.id === 1211 ||
      this.id === 1212) {
    Sound.play('shot_bouncing');
  }

  // Lightning
  if (this.id === 1207 && first) {
    Sound.play('shot_lightning');
  }

  // Sniper
  if (this.id === 1213) {
    Sound.play('shot_railgun_heavy');
  }

  // Medic
  if (this.id === 1214) {
    Sound.play('heal');
  }

  // Healray
  if (this.id === 1215) {
    Sound.play('heal');
  }
};
