/**
 * @file
 * Handles configuration files and user settings
 */
import Event from '../Event';

let serverConfig;
let userSettings = {};

export default {
  VERSION: '1.0.6',
  COLOR_HEALTH_BAR: '#B1233A',
  COLOR_HEALING: 'rgba(80, 250, 150, 1)',
  COLOR_BUFF: 'hsl(70, 100%, 50%)',
  COLOR_DEBUFF: 'hsl(0, 100%, 50%)',
  COLOR_EXPERIENCE: '#ffe926',
  FADE_DURATION: 4000,
  TOWER_KILLSTREAK_DELAY: 5000,
  DURATION_AOE_WINDUP: 1000,
  DURATION_AOE_EFFECT: 250,
  ENDPOINT: 'https://coregrounds.com/',
  LOBBYSERVER: '35.196.132.229:8080',
  PORT: 1337,

  update(data) {
    serverConfig = data;
    Event.fire('config ready');
  },

  get(type) {
    return serverConfig[type];
  },

  query(keys, params, subset = false) {
    const items = [];
    // Convert string keys to array
    if (typeof keys === 'string') keys = [keys];
    keys.forEach((key) => {
      serverConfig[key].forEach((item) => {
        if (meetsParameters(item, params)) {
          items.push(item);
        }
      });
    });
    if (items.length) {
      return subset
        ? items
        : items[0];
    }
    return false;
  },

  getUnitTypeFromId(id) {
    let type = false;
    serverConfig.factories.forEach((item) => {
      if (meetsParameters(item, { id })) {
        if (item.minion) {
          type = 'Minion';
        }
        if (item.tower) {
          type = 'Tower';
        }
      }
    });
    if (!type) {
      serverConfig.abilities.forEach((item) => {
        if (meetsParameters(item, { id })) {
          type = 'Ability';
        }
      });
    }
    if (!type) {
      serverConfig.projectiles.forEach((item) => {
        if (meetsParameters(item, { id })) {
          type = 'Projectile';
        }
      });
    }
    return type;
  },

  User: {
    set(key, val) {
      // Update the value in our reference variable
      userSettings[key] = val;
      // And update the value in our local storage
      window.localStorage.setItem(
        'coregrounds-settings',
        JSON.stringify(userSettings)
      );
    },
    get(key, defaultValue = false) {
      if (userSettings[key] === undefined) {
        return defaultValue;
      }
      return userSettings[key];
    },
  },

};

Event.on('app loaded', () => {
  // Get the user settings from the local storage
  const saved = window.localStorage.getItem('coregrounds-settings');
  if (saved) userSettings = JSON.parse(saved);
});

function meetsParameters(item, params) {
  let result = true;
  Object.keys(params).forEach((property) => {
    if (item[property] !== params[property]) result = false;
  });
  return result;
}
