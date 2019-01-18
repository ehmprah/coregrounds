/**
 * @file
 * API callbacks against the webserver
 */
const request = require('request');
const Logger = require('./logger.js');
const configfile = require('./config.json');

const config =
  process.env.NODE_ENV === 'production'
    ? configfile.production
    : configfile.development;

module.exports = {
  auth(token, successCallback, errorCallback) {
    console.log(token);
    request.post(
      `${config.WEBSERVER}/api/loadplayer`,
      {
        form: {
          key: config.API_KEY,
          sid: token,
        },
      },
      (error, response, body) => {
        if (error) {
          Logger.log(`Error loading player: ${error.toString()}`);
          return errorCallback('Something went wrong. Please try again.');
        }
        try {
          const data = JSON.parse(body);
          if (data.uid === undefined) {
            return errorCallback(data.message);
          }
          successCallback(data);
        } catch (e) {
          Logger.log(`Error loading player: ${e}`);
          return errorCallback('Something went wrong. Please try again.');
        }
      }
    );
  },
};
