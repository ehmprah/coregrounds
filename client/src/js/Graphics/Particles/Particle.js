/**
 * @file
 * A recyclable particle object
 */
import Cache from '../Cache';

export default function Particle() {
  this.x = 0;
  this.y = 0;
  this.vectorX = 0;
  this.vectorY = 0;
  this.angle = 0;
  this.torque = 0;
  this.alpha = 1;
  this.decay = 0;
  this.cacheKey = null;
}

Particle.prototype.reset = function reset() {
  this.x = 0;
  this.y = 0;
  this.vectorX = 0;
  this.vectorY = 0;
  this.angle = 0;
  this.torque = 0;
  this.alpha = 1;
  this.decay = 0;
  this.cacheKey = null;
};

Particle.prototype.shoot = function shoot(maxSpeed = 75, decay = 0.9) {
  // Set a direction and speed at random
  const speed = 25 + (Math.random() * maxSpeed);
  const angle = Math.random() * 2 * Math.PI;
  // Set a decay speed
  this.decay = decay;
  // And translate this info into a movement vector for easier updating
  this.vectorX = speed * Math.cos(angle) * ((Math.random() * 2) - 1);
  this.vectorY = speed * Math.cos(angle) * ((Math.random() * 2) - 1);
  // Give the particle some torque
  this.torque = Math.random() * (Math.PI / 100);
};

Particle.prototype.update = function update(dt) {
  this.alpha -= this.decay * dt;
  this.x += this.vectorX * dt;
  this.y += this.vectorY * dt;
  this.angle += this.torque;
};

Particle.prototype.draw = function draw(context) {
  if (this.cacheKey) {
    const cached = Cache.get(this.cacheKey);
    if (cached) {
      context.save();
      context.globalAlpha = this.alpha;
      context.translate(this.x + 16, this.y + 16);
      context.rotate(this.angle);
      context.drawImage(cached, -16, -16);
      context.restore();
    }
  }
};
