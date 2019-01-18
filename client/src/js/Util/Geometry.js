/**
 * @file Util/Templates.js
 * Collection of helper functions generating HTML
 */
import Game from '../Game/Game';
import Input from '../Input';

export function networkablePosition(tile) {
  let coordinates;

  // If we didn't get a tile supplied, we default to the current cursor position
  if (!tile) {
    const cursor = Input.position();
    coordinates = { x: cursor.x, y: cursor.y };
  } else {
    coordinates = { x: (tile[0] + 0.5) * 128, y: (tile[1] + 0.5) * 128 };
  }

  // Flip them if we're the top right player
  if (Game.playerIndex === 1) {
    coordinates = flipCoordinates(coordinates);
  }

  // Convert to the target coordinate system
  coordinates.x = (coordinates.x / 128) * 1000;
  coordinates.y = (coordinates.y / 128) * 1000;

  // Build bitshifted networkable position
  return coordinates.x | (coordinates.y << 16);
}

export function getX(position) {
  return Game.playerIndex === 1
    ? Math.abs(Math.round(((position & 32767) / 1000) * 128) - 1920)
    : Math.round(((position & 32767) / 1000) * 128);
}

export function getY(position) {
  return Game.playerIndex === 1
    ? Math.abs(Math.round(((position >> 15) / 1000) * 128) - 896)
    : Math.round(((position >> 15) / 1000) * 128);
}

export function getAngle(angle) {
  return Game.playerIndex === 1
    ? angle - Math.PI
    : angle;
}

export function pointIsOnTile(x, y, a) {
  const b = pointToTile(x, y);
  return a[0] === b[0] && a[1] === b[1];
}

export function pointsDistance(ax, ay, bx, by) {
  return Math.sqrt(((ax - bx) * (ax - bx)) + ((ay - by) * (ay - by)));
}

export function pointToTile(x, y) {
  const tile = [];
  tile.push(x / 128 | 0);
  tile.push(y / 128 | 0);
  return tile;
}

export function isOnScreen(box) {
  if (box.x >= -128 && box.x <= 1920 && box.y >= -128 && box.y <= 896) {
    return true;
  }
  return false;
}

export function buildGrid() {
  const grid = [[]];
  for (let y = 0; y <= 6; y++) {
    grid[y] = [];
    for (let x = 0; x <= 14; x++) {
      grid[y][x] = 0;
    }
  }
  return grid;
}

export function invertGrid(grid) {
  grid.forEach((row, index) => {
    grid[index].reverse();
  });
  grid.reverse();
}

export function flipCoordinates(position) {
  position.x = Math.abs(position.x - 1920);
  position.y = Math.abs(position.y - 896);
  return position;
}
