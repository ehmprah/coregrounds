/**
 * @file
 * Streamlines color handling and generates CSS
 *
 * TODO use the new preset colors ingame, too! get rid of chroma dependency subsequently
 */
import chroma from 'chroma-js';
import Config from '../Config/Config';
import Event from '../Event';
import Game from '../Game/Game';
import Session from '../Session/Session';

const chromas = {};

const colorList = {};


// The in game colors
const colors = {
  '-1': 0,
  0: 2400,
  1: 2400,
};

// TODO use this instead of chroma, remove dependency!!!
export function hexToRGBA(hex, alpha = 1) {
  const bigint = parseInt(hex.replace('#', ''), 16);
  return `rgba(${(bigint >> 16) & 255}, ${(bigint >> 8) & 255}, ${bigint & 255}, ${alpha})`;
}

export function colorShade(shade, playerIndex) {
  if (playerIndex === undefined) {
    ({ playerIndex } = Game);
  }
  if (colorList[colors[playerIndex]] && colorList[colors[playerIndex]][shade]) {
    return colorList[colors[playerIndex]][shade];
  }
  return false;
}

export function color(alpha = 1, playerIndex, desaturate = 0) {
  if (playerIndex === undefined) {
    ({ playerIndex } = Game);
  }
  // Get chroma for this player's color
  const clr = chromas[colors[playerIndex]];
  if (clr === undefined) {
    throw new Error(`Could not find color with id ${colors[playerIndex]}`);
  }
  if (desaturate) {
    return clr.desaturate(desaturate).alpha(alpha).css();
  }
  return clr.alpha(alpha).css();
}

export function getHex(colorId) {
  if (colorId === undefined) {
    colorId = Session.getAccount().color;
  }
  if (chromas[colorId] === undefined) {
    // If we have an old color, we just go to the default
    colorId = 2400;
  }
  return chromas[colorId].css();
}

/**
 * Helper function to check if the current match colors are contrast-compatible
 */
function checkContrast() {
  let replace = false;

  // We also replace the color if both players have the same
  if (colors[0] === colors[1]) {
    replace = true;
  } else {
    // Loop through all combos, checking if they contain both colors
    replace = !colorCompatibility(colors[0], colors[1]);
  }

  if (replace) {
    let replacement = 0;
    // eslint-disable-next-line
    for (let color of Config.get('colors')) {
      if (colorCompatibility(colors[Game.playerIndex], color.id)) {
        replacement = color.id;
        break;
      }
    }
    colors[(1 - Game.playerIndex)] = replacement;
  }
}

function colorCompatibility(one, two) {
  let compatible = true;
  Config.get('incompatibleColors').forEach((combo) => {
    if (combo.includes(one) && combo.includes(two)) {
      compatible = false;
    }
  });
  return compatible;
}

Event.on('match colors updated', () => {
  // Get colors from game state
  Game.state.players.forEach((element, index) => {
    colors[index] = element.color;
  });

  // Route that through the contrast handler
  checkContrast();

  // Before telling the rest about it
  Event.fire('match colors ready');
});

/**
 * Generate color CSS once at load
 */
Event.on('session ready', () => {
  let css = '';
  // Add neutral color
  chromas[0] = chroma('#666');
  // Loop over all colors, creating their CSS
  Config.get('colors').forEach((raw) => {
    // Save the color to our hashmap
    colorList[raw.id] = raw;
    // Save the chroma for later TODO REMOVE!
    chromas[raw.id] = chroma(raw.main);
    // Build CSS variations
    css += `
      .text-${raw.id},
      .color-${raw.id} .clr-text,
      .color-${raw.id} a,
      .color-${raw.id} a:visited,
      .color-${raw.id} a.active,
      .color-${raw.id} a:active {
        color: ${raw.main};
      }

      body .color-${raw.id} .box,
      .color-${raw.id} .box {
        background: ${raw.button};
      }

      .color-${raw.id} .box.dark {
        background: ${raw.background};
      }

      .color-${raw.id} progress-bar {
        background: ${raw.background};
      }

      .color-${raw.id} progress-bar bar {
        background: ${raw.second};
      }

      .color-${raw.id} #messages .message {
        background: ${hexToRGBA(raw.main, 0.1)};
      }
    `;
  });

  const style = document.createElement('style');
  style.type = 'text/css';
  style.appendChild(document.createTextNode(css));
  document.head.appendChild(style);
});
