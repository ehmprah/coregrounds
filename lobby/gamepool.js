/**
 * @file
 * Handles game servers and game management
 */
const ServerPool = require('./serverpool.js');
const PlayerPool = require('./playerpool.js');
const Logger = require('./logger.js');
const Game = require('./game.js');
const Util = require('./util.js');

const games = {};

module.exports.newGame = function newGame(type, players, botlevel) {
  const matchId = Util.getId();
  const token = Util.getToken();
  games[matchId] = new Game(matchId, type, token, players, botlevel);
  const region = PlayerPool.getPlayer(players[0]).data.region;
  ServerPool.assignGame(region, matchId);
};

module.exports.recreate = function recreate(match) {
  match.players.forEach((uid) => {
    if (!PlayerPool.getPlayer(uid)) {
      PlayerPool.recreate(uid, match.matchId);
    }
  });
  // In case we get a GameServerMatchList, we recreate games and players to allow for reconnection
  games[match.matchId] = new Game(match.matchId, match.type, match.token, match.players);
  Logger.log(`game recreated (matchId: ${match.matchId})`);
  Util.setId(match.matchId);
};

module.exports.getGame = function getGame(id) {
  if (games[id] && games[id].gc === false) {
    return games[id];
  }
  return false;
};

// Garbage collect the pool every 60 secs
setInterval(() => {
  Object.keys(games).forEach((id) => {
    if (games[id].gc) {
      delete games[id];
    }
  });
}, 60000);
