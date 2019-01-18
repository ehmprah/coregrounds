/**
 * @file
 * Handles games
 */
const Logger = require('./logger.js');
const PlayerPool = require('./playerpool.js');

module.exports = function Game(id, type, token, players, botlevel) {
  this.disconnected = false;
  this.gc = false;
  this.id = id;
  this.ip = '';
  this.type = type;
  this.players = players;
  this.token = token;
  this.botlevel = botlevel;
  // Set players to ingame
  this.players.forEach((uid) => {
    const player = PlayerPool.getPlayer(uid);
    if (player) {
      player.setGame(id);
    }
  });
};

module.exports.prototype.getServerData = function getServerData() {
  const players = [];
  this.players.forEach((uid) => {
    const player = PlayerPool.getPlayer(uid);
    if (player && player.data) {
      // We overwrite the session token here with the match token to allow for
      // reconnection to games across user sessions.
      player.data.sid = this.token + uid;
      players.push(player.data);
    } else {
      Logger.log(`player not found (${uid})`);
    }
  });

  if (this.type === 'bot' || this.type === 'custombot') {
    players.push({ type: 'bot', level: this.botlevel });
  }

  return {
    matchId: this.id,
    type: this.type,
    token: this.token,
    players,
  };
};

module.exports.prototype.setServerIp = function setServerIp(ip) {
  this.ip = ip;
};

module.exports.prototype.start = function start() {
  this.players.forEach((uid) => {
    const player = PlayerPool.getPlayer(uid);
    if (player) {
      PlayerPool.getPlayer(uid).send('ClientGameServerReady', {
        ip: this.ip,
        token: this.token + uid,
      });
    }
  });
  if (this.type === 'bot' || this.type === 'custombot') {
    Logger.log(`Match ${this.id} started on ${this.ip} (${this.type} (${this.botlevel}), ${this.players.toString()})`);
  } else {
    Logger.log(`Match ${this.id} started on ${this.ip} (${this.type}, ${this.players.toString()})`);
  }
};

module.exports.prototype.cancel = function cancel() {
  // Tell players about it
  this.players.forEach((uid) => {
    const player = PlayerPool.getPlayer(uid);
    if (player) {
      player.setGame(0);
      player.send('ClientErrorMessage', 'Could not create game.');
    }
  });
  this.gc = true;
};

module.exports.prototype.isRunning = function isRunning() {
  return this.disconnected === false;
};

module.exports.prototype.setRunning = function setRunning() {
  this.disconnected = false;
};

module.exports.prototype.presumeDisconnect = function presumeDisconnect() {
  this.disconnected = Date.now();
};

module.exports.prototype.destroy = function destroy() {
  this.gc = true;
  this.players.forEach((uid) => {
    const player = PlayerPool.getPlayer(uid);
    if (player) player.setGame(0);
  });
};
