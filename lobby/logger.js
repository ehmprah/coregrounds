/**
 * @file
 * Logging interface
 */
const bot = require('./discordbot.js');

module.exports.log = function log(msg) {
  if (process.env.NODE_ENV !== 'production') {
    // eslint-disable-next-line
    console.log(msg);
  } else {
    // In production, we use slack
    bot.send(msg);
  }
};
