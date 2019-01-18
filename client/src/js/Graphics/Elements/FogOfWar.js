/**
 * @file FogOfWar.js
 * Displays the fog of war on top of the battlefield
 *
 * TODO rewrite this to clean ES6, take care of var handling
 */
import Cache from '../Cache';
import Event from '../../Event';
import Game from '../../Game/Game';
import { buildGrid, invertGrid } from '../../Util/Geometry';

let canvas;
let context;
let grid;
let fullTile;
let semiTile;
let length;
let bitOffset;
let offset;
let i;
let x;
let y;
let previous;

Event.on('cache clear', () => {
  // Create cache for the fog of war layer
  canvas = Cache.create('FogOfWar', 1920, 896);
  context = canvas.getContext('2d');
  // Create cache for fog of war tiles
  fullTile = Cache.set('fullTile', 128, 128, (ctx) => {
    ctx.fillStyle = '#000000';
    ctx.strokeStyle = '#000000';
    ctx.lineWidth = 1;
    ctx.beginPath();
    ctx.tile(0, 0);
    ctx.fill();
    ctx.stroke();
  });
  semiTile = Cache.set('semiTile', 128, 128, (ctx) => {
    ctx.globalAlpha = 0.5;
    ctx.fillStyle = '#000000';
    ctx.strokeStyle = '#000000';
    ctx.lineWidth = 1;
    ctx.beginPath();
    ctx.tile(0, 0);
    ctx.fill();
    ctx.stroke();
  });
});

export default {

  draw(ctx) {
    ctx.drawImage(canvas, 0, 0);
  },

  update(map) {
    if (map.length) {
      length = map.length;
      offset = 0;
      i = 0;
      x = 0;
      y = 0;
      previous = JSON.stringify(grid);
      grid = buildGrid();
      for (; i < 105 && (i / 8) + offset < length; i++) {
        if (grid[y] !== undefined && grid[y][x] !== undefined) {
          bitOffset = (i % 4) * 2;
          grid[y][x] = (map[Math.floor(offset + (i / 4))] >> bitOffset) & 3;
        }
        if (x < 14) {
          x++;
        } else {
          y++;
          x = 0;
        }
      }

      // Invert the grid for the top-right player
      if (Game.playerIndex === 1) {
        invertGrid(grid);
      }

      // Always show the enemy core
      if (grid[0][14] === 0) {
        grid[0][14] = 1;
      }

      // We only redraw the cache if the grid has changed
      if (previous !== JSON.stringify(grid)) {
        context.clearRect(0, 0, 1920, 896);
        y = grid.length;
        while (y--) {
          x = grid[y].length;
          while (x--) {
            if (grid[y][x] === 0) {
              context.drawImage(fullTile, x * 128, y * 128);
            }
            if (grid[y][x] === 1) {
              context.drawImage(semiTile, x * 128, y * 128);
            }
          }
        }
      }
    }
  },

  replace(replacement) {
    grid = replacement;
    context.clearRect(0, 0, 1920, 896);
    y = grid.length;
    while (y--) {
      x = grid[y].length;
      while (x--) {
        if (grid[y][x] === 0) {
          context.drawImage(fullTile, x * 128, y * 128);
        }
        if (grid[y][x] === 1) {
          context.drawImage(semiTile, x * 128, y * 128);
        }
      }
    }
  },
};
