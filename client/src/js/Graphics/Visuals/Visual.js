/**
 * @file
 * A recyclable visual effect object
 */
import { color } from '../Color';
import { getLinear } from '../Counters';
import Cache from '../Cache';

const drawFunctions = {};

export default function Visual() {
  this.type = '';
  this.id = 0;
  this.x = 0;
  this.y = 0;
  this.playerIndex = 0;
  this.done = 0;
  this.a = 0;
  this.b = 0;
  this.c = 0;
}

Visual.prototype.reset = function reset() {
  this.type = '';
  this.id = 0;
  this.x = 0;
  this.y = 0;
  this.playerIndex = 0;
  this.done = 0;
  this.a = 0;
  this.b = 0;
  this.c = 0;
};

Visual.prototype.draw = function draw(context) {
  drawFunctions[this.type](this, context);
};

drawFunctions.unitDrop = function unitDrop(visual, context) {
  const size = 128 * visual.a;
  let sprite = Cache.get(`unitDrop_${visual.id}`);
  if (sprite === false) {
    sprite = Cache.set(`unitDrop_${visual.id}`, 256, 256, (ctx, id) => {
      ctx.unitSprite(id, color(), 192, 0, 256);
    }, visual.id);
  }
  context.drawImage(sprite, visual.x, visual.y, size, size);
};

drawFunctions.wallDrop = function wallDrop(visual, context) {
  const size = 128 * visual.a;
  context.globalAlpha = 0.75;
  context.drawImage(
    Cache.get(`unit_1_${visual.playerIndex}`),
    visual.x,
    visual.y,
    size,
    size
  );
  context.globalAlpha = 1;
};

drawFunctions.ionStrike = function ionStrike(visual, context) {
  context.save();
  // Build path
  context.beginPath();
  context.lineWidth = visual.a;
  context.moveTo(visual.x, 0);
  context.lineTo(visual.x, visual.y);
  // Set shadow
  context.shadowColor = color(1, visual.playerIndex);
  context.shadowBlur = 64;
  // And fill
  context.strokeStyle = color(0.75, visual.playerIndex);
  context.stroke();
  context.restore();
};

drawFunctions.dynablastH = function dynablastH(visual, context) {
  context.save();
  // Build path
  context.beginPath();
  context.lineWidth = visual.a;
  context.moveTo(visual.x - 128, visual.y);
  context.lineTo(visual.x + 128, visual.y);
  // Set shadow
  context.shadowColor = color(1, visual.playerIndex);
  context.shadowBlur = 64;
  // And fill
  context.strokeStyle = color(0.75, visual.playerIndex);
  context.stroke();
  context.restore();
};

drawFunctions.dynablastV = function dynablastV(visual, context) {
  context.save();
  // Build path
  context.beginPath();
  context.lineWidth = visual.a;
  context.moveTo(visual.x, visual.y - 128);
  context.lineTo(visual.x, visual.y + 128);
  // Set shadow
  context.shadowColor = color(1, visual.playerIndex);
  context.shadowBlur = 64;
  // And fill
  context.strokeStyle = color(0.75, visual.playerIndex);
  context.stroke();
  context.restore();
};

drawFunctions.abilityIcon = function abilityIcon(visual, context) {
  // Get/set unit cache
  let sprite = Cache.get(`icon_${visual.id}_${visual.playerIndex}`);
  if (sprite === false) {
    sprite = Cache.set(`icon_${visual.id}_${visual.playerIndex}`, 128, 128, (ctx, id, playerIndex) => {
      ctx.globalAlpha = 0.75;
      ctx.unitSprite(id, color(1, playerIndex), 100);
    }, visual.id, visual.playerIndex);
  }

  // Draw it
  if (visual.a) {
    context.save();
    context.translate(visual.x + 64, visual.y + 64);
    context.rotate(((getLinear() % visual.a) / visual.a) * (2 * Math.PI));
    context.drawImage(sprite, -64, -64);
    context.restore();
  } else {
    context.drawImage(sprite, visual.x, visual.y);
  }
};

drawFunctions.area = function area(visual, context) {
  context.save();
  // If we have a delayed area, we display the max range first
  if (visual.b) {
    context.beginPath();
    context.arc(visual.x, visual.y, visual.b, 0, 2 * Math.PI);
    context.closePath();
    context.lineWidth = 1;
    context.strokeStyle = color(0.75, visual.playerIndex);
    context.stroke();
    // If we have an icon, draw that on top
    if (visual.id) {
      // Get/set unit cache
      let sprite = Cache.get(`icon_${visual.id}_${visual.playerIndex}`);
      if (sprite === false) {
        sprite = Cache.set(`icon_${visual.id}_${visual.playerIndex}`, 128, 128, (ctx, id, playerIndex) => {
          ctx.globalAlpha = 0.75;
          ctx.unitSprite(id, color(1, playerIndex), 100);
        }, visual.id, visual.playerIndex);
      }
      context.drawImage(sprite, visual.x - 64, visual.y - 64);
    }
  }
  // Draw the actual area
  context.globalAlpha = 0.2;
  context.lineWidth = 3;
  context.beginPath();
  context.arc(visual.x, visual.y, visual.a, 0, 2 * Math.PI);
  context.fillStyle = visual.c;
  context.fill();
  context.restore();
};
