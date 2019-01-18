/**
 * @file Util/Game.js
 * Collection of game-related helper functions
 */
import Config from '../Config/Config';
import Session from '../Session/Session';

export function getActiveSkin(unitId) {
  const account = Session.getAccount();
  // Since we use this for rendering, we will supply the non-skinned version
  // as a fallback in case we didn't find an active skin
  let skinId = unitId;
  // Check if we find an active skin for this unit
  Config.get('skins').forEach((skin) => {
    if (account.skins.indexOf(skin.id) >= 0 && skin.unitId === unitId) {
      skinId = skin.id;
    }
  });
  return skinId;
}

export function emptyGrid() {
  const grid = [[]];
  for (let y = 0; y <= 6; y++) {
    grid[y] = [];
    for (let x = 0; x <= 14; x++) {
      grid[y][x] = 0;
    }
  }
  return grid;
}
