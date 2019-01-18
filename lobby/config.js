/**
 * @file
 * Distribute the config file
 */
const fs = require('fs');
const request = require('request');
const Logger = require('./logger.js');
const Cache = require('./cache.js');
const configfile = require('./config.json');

const config =
  process.env.NODE_ENV === 'production'
    ? configfile.production
    : configfile.development;

let localConfig = false;
fs.readFile('./server_config.json', 'utf8', (err, cfg) => {
  if (cfg !== undefined) {
    localConfig = readConfig(cfg);
    if (localConfig) {
      Logger.log('Using local config file');
    }
  }
});

module.exports = {
  local() {
    return localConfig;
  },
  get(callback) {
    // Otherwise we try to get a cached version
    const cfg = Cache.get('config');
    // If we have a cached version, return that immediately
    if (cfg) return callback(cfg);
    // Otherwise just load the config from the server
    request.post(
      `${config.WEBSERVER}/api/config`,
      {
        form: {
          key: config.API_KEY,
        },
      },
      (error, response, body) => {
        if (error)
          return Logger.log(`Error loading config: ${error.toString()}`);
        const parsed = readConfig(body);
        if (parsed) {
          callback(parsed);
        }
      }
    );
  },
};

function readConfig(cfg) {
  let parsed = false;
  try {
    const data = JSON.parse(cfg);
    Cache.set('config', data, 86400);
    parsed = data;
  } catch (e) {
    Logger.log(`Error parsing config: ${e}`);
  }
  return parsed;
}
