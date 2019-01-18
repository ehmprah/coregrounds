/**
 * @file
 * Displays the move overlay for towers
 */
import { buildGrid } from '../../Util/Geometry';
import Game from '../../Game/Game';
import Cache from '../Cache';
import Event from '../../Event';
import Overlays from './Overlays';

let canvas;
let context;
let factory;
let grid = buildGrid();

export default {
  validPosition(x, y) {
    return grid[y][x];
  },
  setFactory(given) {
    factory = given;
  },
};

Event.on('cache clear', () => {
  canvas = Cache.create('MoveOverlay', 1920, 896);
  context = canvas.getContext('2d');
});

Event.on('network update game', update);

function update() {
  // Only update if it's visible
  if (!Overlays.visible) return;
  // Copy the current grid to compare it to the new one
  const previous = JSON.stringify(grid);
  const entities = Object.values(Game.state.entities);
  // Reset grid
  grid = buildGrid();
  // If we have a built tower, take its movement range into account,
  // otherwise we can just take all free walls we have
  if (factory && factory.entity_id && Game.state.entities[factory.entity_id]) {
    // Get tower position
    const x = Math.round(Game.state.entities[factory.entity_id].x / 128);
    const y = Math.round(Game.state.entities[factory.entity_id].y / 128);
    let ex;
    let ey;
    let diffx;
    let diffy;
    // Add all walls to the move overlay
    entities.forEach((entity) => {
      if (entity.player_id === Game.playerIndex && entity.type === 'Wall') {
        ex = Math.round(entity.x / 128);
        ey = Math.round(entity.y / 128);
        diffx = Math.abs(ex - x);
        diffy = Math.abs(ey - y);
        if (diffy <= 2 && diffx <= 2 && !(diffx === 2 && diffy === 2)) {
          grid[ey][ex] = 1;
        }
      }
    });
    // And remove all coordinates with towers
    entities.forEach((entity) => {
      if (entity.player_id === Game.playerIndex && entity.type === 'Tower') {
        grid[Math.round(entity.y / 128)][Math.round(entity.x / 128)] = 0;
      }
    });
  } else {
    // Add all walls to the move overlay
    entities.forEach((entity) => {
      if (entity.player_id === Game.playerIndex && entity.type === 'Wall') {
        grid[Math.round(entity.y / 128)][Math.round(entity.x / 128)] = 1;
      }
    });
    // And remove all coordinates with towers
    entities.forEach((entity) => {
      if (entity.player_id === Game.playerIndex && entity.type === 'Tower') {
        grid[Math.round(entity.y / 128)][Math.round(entity.x / 128)] = 0;
      }
    });
  }

  // We only do a repaint if the grid has changed
  if (previous !== JSON.stringify(grid)) {
    // Get/set the overlay tile cache
    let on = Cache.get('OverlayTile_1');
    if (!on) {
      on = Cache.set('OverlayTile_1', 128, 128, (ctx) => {
        ctx.highlightTile(0, 0);
      });
    }
    context.clearRect(0, 0, 1920, 896);
    // Loop over the grid, painting highlight tiles.
    grid.forEach((row, y) => {
      row.forEach((tile, x) => {
        if (tile && !(y === 0 && x === 14) && !(x === 0 && y === 6)) {
          context.drawImage(on, x * 128, y * 128);
        }
      });
    });
  }
}
