/**
 * @file
 * Extensions for the CanvasRenderingContext2D
 * Fully aware of the fact that this is bad practice, we just do it anyway.
 *
 * Move these functions to the respective classes or a new graphical element!
 * TODO remove all old unused stuff here!
 */
import { color, colorShade } from './Color';
import Config from '../Config/Config';
import Cache from '../Graphics/Cache';
import Game from '../Game/Game';

/**
 * Draw a grayscale version of
 */
CanvasRenderingContext2D.prototype.drawGrayscaled = function(canvas, x, y) {
  let grayscale;
  let context = canvas.getContext('2d');
  let imageData = context.getImageData(0, 0, canvas.width, canvas.height);
  let pixels = imageData.data;
  for (var i = 0, n = pixels.length; i < n; i += 4) {
    grayscale = pixels[i] * .3 + pixels[i + 1] * .59 + pixels[i + 2] * .11;
    pixels[i] = grayscale;
    pixels[i + 1] = grayscale;
    pixels[i + 2] = grayscale;
  }
  this.putImageData(imageData, x, y);1
}

CanvasRenderingContext2D.prototype.statusFX = function statusFX(x, y, fx) {
  if (fx.length) {
    let offset = 0, type, effect;
    for (effect of fx) {
      // Determine effect color
      type = (effect == 101377 ||
              effect == 140577 ||
              effect == 140777 ||
              effect == 140877 ||
              effect == 141077 ||
              effect == 141177 ||
              effect == 141377)
        ? Config.COLOR_BUFF
        : Config.COLOR_DEBUFF;

      // First we get a the proper sprite for the icon
      let sprite = Cache.get(`fx_${effect}_${type}`);
      if (sprite == false) {
        sprite = Cache.set(`fx_${effect}_${type}`, 16, 16,
          function(context, id, type) {
          context.unitSprite(id, type, 16, 0, 16);
        }, effect, type);
      }

      this.drawImage(sprite, x + 40 + offset, y + 33);
      offset += 16;
    }
  }
}

CanvasRenderingContext2D.prototype.userInterfaceBackground = function() {
  this.clearRect(0, 0, 1920, 184);
  let gradient = this.createLinearGradient(0, 0, 0, 180);
  gradient.addColorStop(0, 'black');
  gradient.addColorStop(0.75, 'transparent');
  this.fillStyle = gradient;
  this.fillRect(0, 4, 1920, 176);
}

CanvasRenderingContext2D.prototype.tile = function( x, y ) {
  this.beginPath();
  this.rect( x + 4, y + 4, 120, 120 );
  this.closePath();
}

CanvasRenderingContext2D.prototype.highlightTile = function( x, y, transparent = true ) {
  // For the highlight tiles two things make sense; either full opacity fill
  // color, or just a stroke.
  this.beginPath();
  this.rect( x + 5, y + 5, 118, 118 );
  this.lineWidth = 2;
  // this.fillStyle = 'rgba(34,34,34,1)';
  // this.strokeStyle = 'rgba(34,34,34,1)';
  // this.fill();
  // this.stroke();
  this.fillStyle = color(0.7);
  this.strokeStyle = color(0.7);
  this.fill();
  this.stroke();
}

CanvasRenderingContext2D.prototype.lowlightTile = function( x, y ) {
  this.beginPath();
  this.rect( x + 5, y + 5, 118, 118 );
  this.lineWidth = 2;
  this.fillStyle = 'rgba(0,0,0,0.4)';
  this.strokeStyle = 'rgba(0,0,0,0.6)';
  this.fill();
  this.stroke();
}

CanvasRenderingContext2D.prototype.callToAction = function(x, y) {
  this.beginPath();
    this.moveTo(x, y + 16);
    this.lineTo(x + 32,y);
    this.lineTo(x + 352, y);
    this.lineTo(x + 384, y+ 16);
    this.lineTo(x + 384, y+ 80);
    this.lineTo(x + 352, y+ 96);
    this.lineTo(x + 32, y+ 96);
    this.lineTo(x, y + 80);
    this.lineTo(x, y + 16);
  this.closePath();
}

CanvasRenderingContext2D.prototype.wrapText = function(text, x, y, maxWidth, lineHeight) {
  var words = text.split(' '), line = '', test = '', metrics;
  for(var n = 0; n < words.length; n++) {
    test = line + words[n] + ' ';
    metrics = this.measureText(test);
    if (metrics.width > maxWidth && n > 0) {
      this.strokeFillText(line, x, y, "left");
      line = words[n] + ' ';
      y += lineHeight;
    }
    else {
      line = test;
    }
  }
  this.strokeFillText(line, x, y, "left");
}

CanvasRenderingContext2D.prototype.offStrokeFill = function offStrokeFill() {
  this.lineWidth = 4;
  // this.fillStyle = colorShade('background', Game.playerIndex);
  // this.fill();
  // this.strokeStyle = '#000';
  // this.stroke();
  this.fillStyle = '#0a0a0a';
  this.fill();
  this.strokeStyle = '#000';
  this.stroke();
  return this;
};

CanvasRenderingContext2D.prototype.defaultStrokeFill = function defaultStrokeFill() {
  this.lineWidth = 4;
  this.fillStyle = colorShade('background', Game.playerIndex);
  this.fill();
  this.strokeStyle = '#000';
  this.stroke();
  return this;
};

CanvasRenderingContext2D.prototype.strokeFill = function(stroke, fill) {
  this.fillStyle = '#222';
  this.fill();
  this.fillStyle = fill;
  this.fill();
  this.strokeStyle = '#000';
  this.lineWidth = 6;
  this.stroke();
  this.lineWidth = 4;
  this.strokeStyle = '#222';
  this.stroke();
  this.strokeStyle = stroke;
  this.stroke();
  return this;
}

CanvasRenderingContext2D.prototype.alphaStrokeFill = function(stroke, fill) {
  this.fillStyle = fill;
  this.fill();
  this.strokeStyle = '#000';
  this.lineWidth = 6;
  this.stroke();
  this.lineWidth = 4;
  this.strokeStyle = '#222';
  this.stroke();
  this.strokeStyle = stroke;
  this.stroke();
  return this;
}

CanvasRenderingContext2D.prototype.strokeFillText = function(text, x, y, align = 'center', fill = '#dbdbdb') {
  this.lineJoin = 'round';
  this.strokeStyle = '#000';
  this.lineWidth = 4;
  this.fillStyle = fill;
  this.textAlign = align;
  this.textBaseline = "middle";
  this.strokeText(text,x,y);
  this.fillText(text,x,y);
  return this;
}

CanvasRenderingContext2D.prototype.customRectagon = function(x, y, width, height, radius) {
  this.beginPath();
    this.moveTo(x + radius, y);
    this.lineTo(x + width - radius, y);
    this.lineTo(x + width, y + radius);
    this.lineTo(x + width, y + height - radius);
    this.lineTo(x + width - radius, y + height);
    this.lineTo(x + radius, y + height);
    this.lineTo(x, y + height - radius);
    this.lineTo(x, y + radius);
    this.lineTo(x + radius, y);
  this.closePath();
}

CanvasRenderingContext2D.prototype.rectagon = function(x, y) {
  x += 10;
  y += 10;
  var width = 108, height = 108, radius = 12;
  this.beginPath();
    this.moveTo(x + radius, y);
    this.lineTo(x + width - radius, y);
    this.lineTo(x + width, y + radius);
    this.lineTo(x + width, y + height - radius);
    this.lineTo(x + width - radius, y + height);
    this.lineTo(x + radius, y + height);
    this.lineTo(x, y + height - radius);
    this.lineTo(x, y + radius);
    this.lineTo(x + radius, y);
  this.closePath();
}

CanvasRenderingContext2D.prototype.septagon = function(x, y) {
  x += 10;
  y += 10;
  var width = 108, height = 108, radius = 16;
  this.beginPath();
    this.moveTo(x+width*3/4+8,y);
    this.lineTo(x + width, y + height - radius*1.5);
    this.lineTo(x + width - radius, y + height);
    this.lineTo(x + radius, y + height);
    this.lineTo(x, y + height - radius*1.5);
    this.lineTo(x+width*1/4-8,y);
  this.closePath();
}

CanvasRenderingContext2D.prototype.octagon = function(x, y) {
  var size = 54;
  x += 10 + size;
  y += 10 + size;
  this.beginPath();
  this.moveTo(x + size * Math.cos(0), y + size * Math.sin(0));
  for (var i = 1; i <= 8;i += 1) {
    this.lineTo(x + size * Math.cos(i * 2 * Math.PI / 8), y + size * Math.sin(i * 2 * Math.PI / 8));
  }
  this.closePath();
}

CanvasRenderingContext2D.prototype.healthBar = function(x, y, percentage) {
  this.lineCap = 'square';
  this.lineWidth = 5;
  this.beginPath();
  this.moveTo(x + 40, y + 24);
  this.lineTo(x + 88, y + 24);
  this.strokeStyle = "#000";
  this.stroke();
  this.lineWidth = 3;
  this.beginPath();
  this.moveTo(x + 40, y + 24);
  this.lineTo(x + 40 + 48 * percentage, y + 24);
  this.strokeStyle = Config.COLOR_HEALTH_BAR;
  this.stroke();
}

CanvasRenderingContext2D.prototype.drawReload = function(x, y, percentage) {
  percentage = percentage < 0 ? 0 : percentage;
  percentage = percentage > 1 ? 1 : percentage;
  this.beginPath();
  this.moveTo(x + 40, y + 29);
  this.lineTo(x + 40 + 48 * percentage, y + 29);
  this.lineCap = 'square';
  this.lineWidth = 5;
  this.strokeStyle = "#000";
  this.stroke();
  this.lineWidth = 3;
  this.strokeStyle = "#aaa";
  this.stroke();
}

CanvasRenderingContext2D.prototype.chainBonus = function(x, y, chain) {
  // Get / set the chain cache
  let sprite = Cache.get(`chain_${chain}`);
  if (sprite == false) {
    sprite = Cache.set(`chain_${chain}`, 128, 128, function(context, chain) {
      context.lineWidth = 1;
      context.strokeStyle = '#000';
      context.fillStyle = '#aaa';
      let offset = 64 - chain * 2.5;
      while (chain--) {
        context.beginPath();
        context.rect(offset, 18, 4, 3);
        context.fill();
        context.stroke();
        offset += 5;
      }
    }, chain);
  }
  // Draw the cached chain
  this.drawImage(sprite, x, y);
}

CanvasRenderingContext2D.prototype.cooldownRectagon = function(x, y, percentage, reverse) {
  reverse = reverse || 0;
  var width = 112, height = 112;
  this.save();
  this.rectagon(x,y);
  this.clip();
  x += 8;
  y += 8;
  this.beginPath();
    this.moveTo(x+(width/2),y+(height/2));
    if(reverse) {
      this.arc(x+(width/2),y+(height/2), width, 1.5 * Math.PI, 1.5 * Math.PI - 2 * percentage * Math.PI, false);
    } else {
      this.arc(x+(width/2),y+(height/2), width, 1.5 * Math.PI + 2 * percentage * Math.PI, 1.5 * Math.PI, false);
    }
  this.closePath();
    this.fillStyle = 'rgba(20,20,20,0.7)';
    this.fill();
  this.restore();
}

CanvasRenderingContext2D.prototype.cooldownRect = function(x, y, percentage) {
  var width = 112, height = 112;
  x += 8;
  y += 8;
  this.save();
  this.rect(x, y, width, height);
  this.clip();
  this.beginPath();
  this.moveTo(x + (width / 2), y + (height / 2));
  this.arc(x+(width/2),y+(height/2), width, 1.5 * Math.PI + 2 * percentage * Math.PI, 1.5 * Math.PI, false);
  this.closePath();
  this.fillStyle = 'rgba(0, 0, 0, 0.7)';
  this.fill();
  this.restore();
}

CanvasRenderingContext2D.prototype.circle = function(x, y) {
  var width = 112, height = 112;
  x += 8;
  y += 8;
  this.beginPath();
  this.moveTo(x + (width / 2), y + (height / 2));
  this.arc(x + (width / 2), y + (height / 2), width / 2, 0, 2 * Math.PI, false);
  this.closePath();
}

CanvasRenderingContext2D.prototype.cooldownOctagon = function(x, y, percentage) {
  var width = 112, height = 112;
  this.save();
  this.octagon(x,y);
  this.clip();
  x += 8;
  y += 8;
  this.beginPath();
    this.moveTo(x+(width/2),y+(height/2));
    this.arc(x+(width/2),y+(height/2), width, 1.5 * Math.PI + 2 * percentage * Math.PI, 1.5 * Math.PI, false);
  this.closePath();
    this.fillStyle = 'rgba(20,20,20,0.7)';
    this.fill();
  this.restore();
}

CanvasRenderingContext2D.prototype.cooldownSeptagon = function(x, y, percentage) {
  var width = 112, height = 112;
  this.save();
  this.septagon(x,y);
  this.clip();
  x += 8;
  y += 8;
  this.beginPath();
    this.moveTo(x+56,y+56);
    this.arc(x+56,y+56, width, 1.5 * Math.PI + 2 * percentage * Math.PI, 1.5 * Math.PI, false);
  this.closePath();
    this.fillStyle = 'rgba(20,20,20,0.7)';
    this.fill();
  this.restore();
}

CanvasRenderingContext2D.prototype.drawSeconds = function(x, y, cooldown) {
  var width = 112, height = 112;
  x += 8;
  y += 8;
  this.font = '28px "Exo 2"';
  if(cooldown < 1) {
    this.strokeFillText(cooldown.toFixed(1), x + 56, y + 56);
  } else {
    this.strokeFillText((cooldown | 0), x + 56, y + 56);
  }
}

CanvasRenderingContext2D.prototype.factoryHighlight = function(x, y) { //@cachable
  this.beginPath();
  this.moveTo(x+64,y-4);
  this.lineTo(x+80,y+12);
  this.lineTo(x+48,y+12);
  this.lineTo(x+64,y-4);
  this.fillStyle = color(0.5);
  this.fill();
  this.strokeStyle = '#000';
  this.lineWidth = 1;
  this.stroke();
}

CanvasRenderingContext2D.prototype.level = function( x, y, lvl, progress ) {
  let fill = color();
  x += 19;
  y += 112;
  // x += 64;
  // y += 120;
  this.lineWidth = 3;
  this.beginPath();
  this.moveTo( x, y );
  this.arc( x, y, 19, 0, 2 * Math.PI );
  this.closePath();
  this.strokeStyle = '#000';
  this.stroke();
  this.fillStyle = fill;
  this.fill();
  this.beginPath();
  this.moveTo( x, y );
  if(lvl == 20) {
    this.arc( x, y, 19, 1.5 * Math.PI, 1.5 * Math.PI );
  } else {
    this.arc( x, y, 19, 1.5 * Math.PI - (1 - progress) * 2 * Math.PI, 1.5 * Math.PI );
  }
  this.closePath();
  this.fillStyle = "#000";
  this.fill();
  this.beginPath();
  this.arc( x, y, 15, 0, 2 * Math.PI );
  this.closePath();
  this.strokeStyle = '#000';
  this.stroke();
  this.fillStyle = '#222';
  this.fill();
  this.fillStyle = color(0.15);
  this.fill();
  this.font = '24px "Exo 2"';
  this.strokeFillText(lvl, x, y, 'center', fill);
}

CanvasRenderingContext2D.prototype.drawStacks = function(x, y, stacks, progress) {
  // Move coordinates to the bottom right of the tile
  x += 90;
  y += 90;
  // We only display a progress if we have it
  if (progress) {
    this.save();
    let angle = 1.5 * Math.PI + progress * 2 * Math.PI;
    // Clip a region between two partial circles
    this.beginPath();
    this.moveTo(x, y);
    this.arc(x, y, 18, 1.5 * Math.PI, angle, false);
    this.moveTo(x, y);
    this.arc(x, y, 16, 1.5 * Math.PI, angle, false);
    // "evenodd" gets us the difference between the two circles
    this.clip("evenodd");
    // We now paint the actual circle
    this.beginPath();
    this.arc(x, y, 20, 0, 2 * Math.PI);
    this.fillStyle = '#dbdbdb';
    this.fill();
    this.restore();
  }
  // Draw the stack amount
  this.font = '25px "Exo 2"';
  this.strokeFillText(stacks, x, y, 'center', '#dbdbdb');
}
