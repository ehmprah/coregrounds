/**
 * @file
 * Handles assets and animations
 */
import * as sprites from '../../img/units/index';
import { getActiveSkin } from '../Util/Game';
import Config from '../Config/Config';
import { getHex } from '../Graphics/Color';

export default {
  withBase(id, color = false, linkToLayer = -1, skinned = true, cls = '') {
    // Build link to add to the sprite
    const link = linkToLayer >= 0
      ? `onclick="Screens.show('game-guide-detail', ${linkToLayer}, ${id})"`
      : '';
    // Get default color if it wasn't provided
    if (!color) color = getHex();
    // Get the active skin for the wall base if this should be skinned
    const wallSpriteId = skinned ? getActiveSkin(1) : 1;
    // We try towers / minions first
    let item = Config.query('factories', { id });
    if (item) {
      return `
        <sprite class="${linkToLayer >= 0 ? 'clickable' : ''} ${item.tower ? 'tower' : 'minion'} ${cls}" ${link}>
          ${item.tower ? svg(wallSpriteId, color) : '<svg xmlns="http://www.w3.org/2000/svg" width="128" height="128"></svg>'}
          ${svg(item.id, color)}
        </sprite>
      `;
    }
    // Otherwise we'll try an ability
    item = Config.query('abilities', { id });
    if (item) {
      return `
        <sprite class="${linkToLayer >= 0 ? 'clickable' : ''} ability ${cls}" ${link}>
          <svg xmlns="http://www.w3.org/2000/svg" width="128" height="128" viewBox="0 0 33.867 33.867">
            <g transform="translate(0 -263.133)" stroke-width="1.587" stroke-linejoin="round">
              <circle cx="16.933" cy="280.067" r="14.275" fill="#222" stroke="#000"/>
              <circle cx="16.933" cy="280.067" r="12.745" fill="none" stroke="#333"/>
              <circle cx="16.933" cy="280.067" r="14.275" fill="${color}" fill-opacity="0.1"/>
            </g>
          </svg>
          ${svg(item.id, color)}
        </sprite>
      `;
    }
    // Otherwise we'll just render the given sprite
    return `
      <sprite>
        ${svg(id, color)}
      </sprite>
    `;
  },
};

CanvasRenderingContext2D.prototype.unitSprite = function unitSprite(
  id,
  color,
  size = 128,
  rotate = 0,
  base = 128,
  shadow = false
) {
  const img = new Image();

  if (rotate) {
    img.onload = () => {
      const offset = (base - size) / 2;
      const translate = size / 2;
      this.translate(offset + translate, offset + translate);
      this.rotate(rotate);
      this.drawImage(img, -translate, -translate, size, size);
    };
  } else {
    const offset = (base - size) / 2;
    img.onload = () => {
      // Add shadow for projectiles
      if (shadow) {
        this.shadowColor = color;
        this.shadowBlur = 16;
        this.drawImage(img, offset, offset, size, size);
        this.drawImage(img, offset, offset, size, size);
        this.drawImage(img, offset, offset, size, size);
      }
      this.drawImage(img, offset, offset, size, size);
    };
  }

  if (sprites[`s${id}`] === undefined) {
    throw new Error(`Sprite with id ${id} not found`);
  }

  const sprite = sprites[`s${id}`].replace(/#1E70B5/gi, `${color}`);

  img.src = `data:image/svg+xml;base64,${window.btoa(sprite)}`;
};

function svg(id, color = false) {
  let sprite = sprites[`s${id}`];
  if (color) {
    sprite = sprite.replace(/#1E70B5/gi, `${color}`);
  }
  return sprite;
}
