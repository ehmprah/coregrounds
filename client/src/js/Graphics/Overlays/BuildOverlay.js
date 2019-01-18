/**
 * @file
 * Displays the build overlay for walls
 */
import { buildGrid, invertGrid } from '../../Util/Geometry';
import Cache from '../Cache';
import Event from '../../Event';
import Game from '../../Game/Game';

let canvas;
let context;
let grid = buildGrid();

Event.on('cache clear', () => {
  // Create cache for the build overlay
  canvas = Cache.create('BuildOverlay', 1920, 896);
  context = canvas.getContext('2d');
  // Cache the locked tile
  Cache.set('LockedTile', 128, 128, (ctx) => {
    ctx.fillStyle = 'rgba(40, 0, 0, 0.8)';
    ctx.tile(0, 0);
    ctx.fill();
    ctx.drawIcon('lock', 'rgba(60, 30, 30, 0.8)');
  });
});

Event.on('match colors ready', () => {
  Cache.set('OverlayTile_1', 128, 128, (ctx) => {
    ctx.highlightTile(0, 0);
  });
});

export default {

  validPosition(x, y) {
    return grid[y][x];
  },

  update(map) {
    if (map.length) {
      // Keep a stringified copy of the current grid to check wether we need to redraw
      const previous = JSON.stringify(grid);

      // Build the new grid
      grid = buildGrid();
      const { length } = map;
      for (let i = 0, x = 0, y = 0; i < 105 && i / 8 < length; i++) {
        if (grid[y] !== undefined && grid[y][x] !== undefined) {
          grid[y][x] = (map[Math.floor(i / 8)] >> (i % 8)) & 1;
        }
        if (x < 14) {
          x += 1;
        } else {
          y += 1;
          x = 0;
        }
      }

      // Invert the grid for the top-right player
      if (Game.playerIndex === 1) {
        invertGrid(grid);
      }

      // We only redraw the cache if the grid has changed
      if (previous !== JSON.stringify(grid)) {
        redraw();
      }
    }
  },

  replace(replacement) {
    grid = replacement;
    redraw();
  },
};

function redraw() {
  const settled = Game.state.players[Game.playerIndex].settled;
  context.clearRect(0, 0, 1920, 896);
  const overlayTile = Cache.get('OverlayTile_1');
  grid.forEach((row, y) => {
    row.forEach((cell, x) => {
      if (cell) {
        if (!(y === 0 && x === 14) && !(x === 0 && y === 6)) {
          if (settled || x < 7) {
            context.drawImage(overlayTile, x * 128, y * 128);
          }
        }
      }
    });
  });

  // Display settled flag
  if (!settled) {
    // Draw lock icons across locked area
    const lockedTile = Cache.get('LockedTile');
    for (let x = 7; x < 15; x++) {
      for (let y = 0; y < 8; y++) {
        context.drawImage(lockedTile, x * 128, y * 128);
      }
    }
  }
}
