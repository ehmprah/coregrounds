/**
 * Core.js
 * Renders cores
 *
 * @todo REWRITE TO ES6
 * @todo render the layers as svgs, cache layers and render these.
 *       (sprite already added to sprites.svg)
 * @todo rewrite the whole timing thing.
 */
import { getLinear } from '../Counters';
import { color } from '../Color';

export default {

  draw: function(context, x, y, player_id) {

    x += 64;
    y += 64;
    var ticks = getLinear()/20, start = 0, i;
    context.lineWidth = 2;

    for(i = 1; i <= 2; i++) {
      context.beginPath();
      context.moveTo(x,y);
      context.fillStyle = color(0.4, player_id);
      start = ((ticks % 512)/512) * 2 * Math.PI - i*Math.PI;
      context.arc(x, y, 48, start, start + 1.8, false);
      context.closePath();
      context.fill();
      context.strokeStyle = 'rgba(0,0,0,0.4)';
      context.stroke();
    }

    for(i = 0.333; i <= 1; i += 0.333) {
      context.beginPath();
      context.moveTo(x, y);
      context.fillStyle = color(0.4, player_id);
      start = (1-((ticks % 256)/256)) * 2 * Math.PI - i*Math.PI;
      context.arc(x, y, 48, start, start + 0.7, false);
      context.closePath();
      context.fill();
      context.strokeStyle = 'rgba(0,0,0,0.4)';
      context.stroke();
    }

    for(i = 0.5; i < 2; i += 0.5) {
      context.beginPath();
      context.moveTo(x, y);
      context.fillStyle = color(0.4, player_id);
      start = ((ticks % 256)/256) * 2 * Math.PI - i*Math.PI;
      context.arc(x, y, 48, start, start + 0.7, false);
      context.closePath();
      context.fill();
      context.strokeStyle = 'rgba(0,0,0,0.4)';
      context.stroke();
    }

    context.beginPath();
    context.arc(x, y, 26, 0, 2 * Math.PI);
    context.closePath();
    context.fillStyle = '#111';
    context.fill();
  },

  healthBar: function(context, x, y, player_id, percentage) {

    x += 64;
    y += 64;

    context.lineWidth = 2;
    context.beginPath();
    context.moveTo(x, y);
    context.arc(x, y, 18, 0, 2 * Math.PI);
    context.closePath();
    context.strokeStyle = '#000';
    context.stroke();
    context.fillStyle = "#d00031";
    context.fill();
    context.beginPath();
    context.moveTo(x, y);
    context.arc(x, y, 18, 1.5 * Math.PI - percentage * 2 * Math.PI, 1.5 * Math.PI);
    context.closePath();
    context.fillStyle = color(1, player_id);
    context.fill();
    if(percentage < 1) {
      context.stroke();
    }
    context.beginPath();
    context.arc(x, y, 10, 0, 2 * Math.PI);
    context.closePath();
    context.strokeStyle = '#000';
    context.stroke();
    context.fillStyle = '#111';
    context.fill();
  },

};