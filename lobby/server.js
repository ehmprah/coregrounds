/**
 * @file
 * Gameserver handling
 */
const request = require('request');
const Config = require('./config.js');
const GamePool = require('./gamepool.js');
const Logger = require('./logger.js');
const configfile = require('./config.json');
const GCE = require('./gce.js');

const config = process.env.NODE_ENV === 'production'
  ? configfile.production
  : configfile.development;

function Server(ws, ip) {
  this.gc = false;
  this.ready = false;
  this.restart = false;
  this.healthy = true;
  this.priority = 1;
  this.region = 'local';
  this.ws = ws;
  this.ip = ip;
  this.games = [];

  // Set the region
  GCE.setRegion(this);

  // Send the config to the game server
  Config.get((cfg) => {
    this.send('GameServerSetConfig', cfg);
  });

  // Handle incoming messages
  this.ws.on('message', (payload) => {
    try {
      const msg = JSON.parse(payload);
      if (msg.event === 'GameServerReady') {
        this.ready = true;
        Logger.log(`Server ${this.ip} in ${this.region.toUpperCase()} ready`);
        this.send('GameServerGetMatchList');
      } else if (msg.event === 'GameServerGameConfirm') {
        GamePool.getGame(msg.data.matchId).start(this.ip);
      } else if (msg.event === 'GameServerGameFinished') {
        this.finishGame(msg.data);
      } else if (msg.event === 'GameServerGameRefuse' || msg.event === 'GameServerGameCancel') {
        const game = GamePool.getGame(msg.data.matchId);
        if (game) game.cancel();
      } else if (msg.event === 'GameServerHeartBeatGame') {
        const game = GamePool.getGame(msg.data.matchId);
        if (game) {
          if (msg.data.running) {
            game.setRunning();
          } else {
            this.removeGame(msg.data.matchId);
            game.destroy();
          }
        }
      } else if (msg.event === 'GameServerMatchList') {
        if (msg.data.matches) {
          msg.data.matches.forEach((match) => {
            const game = GamePool.getGame(match.matchId);
            if (game) {
              game.setRunning();
            } else {
              GamePool.recreate(match);
              this.games.push(match.matchId);
              GamePool.getGame(match.matchId).setServerIp(this.ip);
            }
          });
        }
      }
    } catch (e) {
      Logger.log(e);
    }
  });

  // Handle closed connections
  this.ws.on('close', () => {
    this.games.forEach((id) => {
      const game = GamePool.getGame(id);
      if (game) game.destroy();
    });
    // Mark this server for garbage collection
    this.ready = false;
    this.gc = true;
    Logger.log(`Server ${this.ip} disconnected`);
  });
}

Server.prototype.send = function send(evt, data) {
  if (this.ws && this.ws.readyState === 1) this.ws.sendJSON(evt, data);
};

Server.prototype.scheduleRestart = function scheduleRestart() {
  this.restart = true;
};

Server.prototype.addGame = function addGame(matchId) {
  this.games.push(matchId);
  const game = GamePool.getGame(matchId);
  game.setServerIp(this.ip);
  this.send('GameServerGameCreate', game.getServerData());
};

Server.prototype.removeGame = function removeGame(matchId) {
  const index = this.games.indexOf(matchId);
  if (index >= 0) this.games.splice(index, 1);
};

Server.prototype.checkGameHealth = function checkGameHealth() {
  this.games.forEach((matchId) => {
    const game = GamePool.getGame(matchId);
    if (game) {
      if (game.isRunning()) {
        game.presumeDisconnect();
        this.send('GameServerHeartBeatGame', { matchId });
      } else {
        Logger.log(`removed no longer running game ${matchId}`);
        this.removeGame(matchId);
        game.destroy();
      }
    } else {
      Logger.log(`removed stale game ${matchId}`);
      this.removeGame(matchId);
    }
  });
};

Server.prototype.load = function load() {
  return this.games.length / config.GCE.MAX_GAMES_PER_INSTANCE;
};

Server.prototype.finishGame = function finishGame(data) {
  // Remove the game from this server
  this.removeGame(data.matchId);
  // Mark the game itself for deletion
  const game = GamePool.getGame(data.matchId);
  if (game) game.destroy();
  // Send match statistics to web server
  request.post(`${config.WEBSERVER}/api/match`, {
    form: {
      key: config.API_KEY,
      match: data.match,
    },
  }, (error, response, body) => {
    if (error) return Logger.log('Error saving game statistics: ', error);
    try {
      const permanentId = JSON.parse(body).match_id;
      Logger.log(`Match ${data.matchId} finished: https://coregrounds.com/match/${permanentId}`);
      this.send('GameServerSendMatchId', {
        matchId: data.matchId,
        permanentId,
      });
    } catch (e) {
      Logger.log('Error saving game statistics: ', e);
    }
  });
};

module.exports = Server;
