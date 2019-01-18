/**
 * @file
 * Player class
 */
const Logger = require('./logger.js');
const PlayerPool = require('./playerpool.js');
const ServerPool = require('./serverpool.js');
const GamePool = require('./gamepool.js');
const DiscordBot = require('./discordbot.js');

function Player() {
  this.gc = false;
  this.game = 0;
  this.ws = false;
  this.data = false;
  // Matchmaking
  this.private = 0;
  this.privateType = '';
  this.queue = '';
  this.queueJoined = 0;
}

Player.prototype.update = function update(ws, data) {
  this.gc = false;
  this.ws = ws;
  this.data = data;
  // Handle incoming messages
  this.ws.on('message', (payload) => {
    try {
      const msg = JSON.parse(payload);
      if (msg.event === 'ClientPing') {
        if (ServerPool.hasReadyServer(this.data.region)) {
          this.ws.sendJSON('ClientPong');
        }
      } else if (msg.event === 'ClientStartBotGame') {
        let botlevel = this.data.season.bot.rank + 1;
        if (botlevel > 100) botlevel = 100;
        GamePool.newGame('bot', [this.data.uid], botlevel);
      } else if (msg.event === 'ClientStartCustomBotGame') {
        const botlevel = parseInt(msg.data.botlevel, 10);
        if (Number.isInteger(botlevel) && botlevel > 0 && botlevel <= 100) {
          GamePool.newGame('custombot', [this.data.uid], botlevel);
        } else {
          this.send('ClientErrorMessage', 'Invalid bot level.');
        }
      } else if (msg.event === 'ClientJoinQueue') {
        this.joinQueue(msg.data);
      } else if (msg.event === 'ClientQuitQueue') {
        this.quitQueue();
      } else if (msg.event === 'ClientPrivateCreate') {
        this.hostPrivate(msg.data);
      } else if (msg.event === 'ClientPrivateCancel') {
        this.cancelPrivate();
      } else if (msg.event === 'ClientPrivateJoin') {
        this.joinPrivate(msg.data);
      }
    } catch (e) {
      Logger.log(e);
    }
  });

  // Handle closed connections
  this.ws.on('close', () => {
    if (!this.game) {
      this.private = 0;
      this.queue = '';
      this.gc = true;
    }
  });
};

Player.prototype.send = function send(evt, data) {
  if (this.ws && this.ws.readyState === 1) this.ws.sendJSON(evt, data);
};

Player.prototype.joinQueue = function joinQueue(type) {
  if (type === 'casual' || type === 'ranked') {
    this.queue = type;
    this.queueJoined = Date.now();
    this.send('ClientQueueConfirm', PlayerPool.getEstimatedQueueTime(type));
    // Send message to #matchmaking to allow for quick matchmaking
    if (process.env.NODE_ENV === 'production') {
      const date = new Date();
      const time = `${strPad(date.getHours())}:${strPad(date.getMinutes())} UTC:`;
      DiscordBot.send(
        `${time} A player joined the ${type} queue (${this.data.region})`,
        '470898279811645460'
      );
    }
  } else {
    this.send('ClientErrorMessage', 'Invalid queue type.');
  }
};

Player.prototype.quitQueue = function quitQueue() {
  this.queue = 0;
};

Player.prototype.hostPrivate = function hostPrivate(type) {
  if (type === 'private' || type === 'random') {
    this.private = PlayerPool.getPrivateMatchId();
    this.privateType = type;
    this.send('ClientPrivateMatchReady', this.private);
  } else {
    this.send('ClientErrorMessage', 'Invalid match type.');
  }
};

Player.prototype.cancelPrivate = function cancelPrivate() {
  this.private = 0;
};

Player.prototype.joinPrivate = function hostPrivate(data) {
  if (data.mid && /^\d{4}$/.test(data.mid)) {
    const uid = PlayerPool.getPlayerByPrivateMatchId(data.mid);
    if (uid) {
      return GamePool.newGame('private', [uid, this.data.uid]);
    }
    return this.send('ClientPrivateMatchNotFound');
  }
  this.send('ClientPrivateMatchIdInvalid');
};

Player.prototype.setGame = function setGame(id) {
  this.game = id;
  // Upon deletion of games, we mark player objects for delection if they're not connected
  if (!this.game && (!this.ws || this.ws.readyState !== 1)) {
    this.gc = true;
  }
};

module.exports = Player;

function strPad(n) {
  return String(`00${n}`).slice(-2);
}
