/**
 * @file
 * Pooling for players
 */
const ServerPool = require('./serverpool.js');
const GamePool = require('./gamepool.js');
const Player = require('./player.js');
const Config = require('./config.js');
const API = require('./api.js');

const players = {};
const queueTimeEstimate = {
  casual: 600,
  ranked: 1800,
};

module.exports.addPlayer = function addPlayer(ws, token) {
  // Authenticate the user against the webserver
  API.auth(token, (data) => {
    // If we don't have any game servers, we don't even auth
    if (!ServerPool.hasReadyServer(data.region)) {
      const msg = 'No game servers available in your region. Please try again in a few minutes.';
      return closeSocketWithError(ws, msg);
    }
    // If we have an existing session we check for running games
    if (players[data.uid] !== undefined) {
      // Update the data here to allow for players recreated via GameServerMatchList
      players[data.uid].update(ws, data);
      if (players[data.uid].game) {
        const game = GamePool.getGame(players[data.uid].game);
        if (game) {
          ws.sendJSON('ClientGameServerReady', {
            ip: game.ip,
            token: game.token + data.uid,
          });
          return ws.terminate();
        }
      }
    } else {
      players[data.uid] = new Player();
      players[data.uid].update(ws, data);
    }
    ws.sendJSON('ClientLoginSuccessful', { config: Config.local() });
  }, (error) => {
    closeSocketWithError(ws, error);
  });
};

module.exports.recreate = function recreate(uid) {
  players[uid] = new Player();
};

module.exports.getPlayer = function getPlayer(id) {
  if (players[id]) {
    return players[id];
  }
  return false;
};

module.exports.getPlayerByPrivateMatchId = function getPlayerByPrivateMatchId(matchId) {
  let uid = false;
  Object.keys(players).forEach((id) => {
    if (players[id].private === matchId) {
      uid = id;
    }
  });
  return uid;
};

module.exports.getPrivateMatchId = function getPrivateMatchId() {
  let id = '';
  const possible = '123456789';
  for (let t = 0; t < 4; t++) {
    id += possible.charAt(Math.floor(Math.random() * possible.length));
  }
  if (module.exports.getPlayerByPrivateMatchId(id) !== false) {
    return module.exports.getPrivateMatchId();
  }
  return id;
};

module.exports.getEstimatedQueueTime = function getEstimatedQueueTime(type) {
  return queueTimeEstimate[type];
};

// Garbage collect the pool every 60 secs
setInterval(() => {
  Object.keys(players).forEach((id) => {
    if (players[id].gc) {
      delete players[id];
    }
  });
}, 60000);

// Matchmaking runs every second
setInterval(() => {
  Object.keys(players).forEach((uid) => {
    const player = players[uid];
    if (player.queue && !player.game) {
      const queue = player.queue;
      const opponentId = findMatch(player);
      if (opponentId) {
        GamePool.newGame(queue, [opponentId, uid]);
      }
    }
  });
}, 1000);

function findMatch(player) {
  let match = false;
  const rank = player.data.season[player.queue].rank;
  const elapsed = Math.floor((Date.now() - player.queueJoined) / 1000);
  // The default max rank difference
  let closest = 5;
  // We increase the max rank difference by 1 per second spent in queue
  closest += elapsed;
  // Loop over the queue, trying to find the player which is the closest match
  Object.keys(players).forEach((uid) => {
    const opponent = players[uid];
    if (fitsQueue(player, opponent) && fitsRegion(player, opponent, elapsed)) {
      // First we calculate the rank difference to the potential opponent
      const diff = Math.abs(rank - opponent.data.season[player.queue].rank);
      // And if the difference is below the matching threshold, we have a match
      if (diff < closest) {
        closest = diff;
        match = opponent.data.uid;
      }
    }
  });
  // Remove matched players from queue
  if (match) {
    player.queue = '';
    players[match].queue = '';
  }
  return match;
}

function fitsQueue(player, opponent) {
  return player.queue === opponent.queue && player.data.uid !== opponent.data.uid;
}

function fitsRegion(player, opponent, elapsed) {
  return (player.data.crossregion && elapsed > 30) ||
         player.data.region === opponent.data.region;
}

function closeSocketWithError(ws, error) {
  ws.send(JSON.stringify({
    event: 'ClientErrorMessage',
    data: error,
  }));
  ws.terminate();
}
