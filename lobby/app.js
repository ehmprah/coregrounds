/**
 * @file
 * The mothership lobby server
 */
const WebSocket = require('ws');
const Logger = require('./logger.js');
const PlayerPool = require('./playerpool.js');
const ServerPool = require('./serverpool.js');
require('./gce.js');

// Create websocket server
const wss = new WebSocket.Server({ port: 8080 });

// Create connection handling
wss.on('connection', (ws, req) => {
  const ip = req.connection.remoteAddress.replace(/^.*:/, '');

  // Add JSON syntax sugar
  ws.sendJSON = function sendJSON(event, data) {
    if (this.readyState === WebSocket.OPEN) {
      const payload = { event };
      if (data) payload.data = data;
      this.send(JSON.stringify(payload));
    }
  };

  ws.on('message', (payload) => {
    if (ws.isUpgraded) return;
    try {
      const msg = JSON.parse(payload);
      if (msg.event === 'GameServerRegister') {
        ServerPool.addServer(ws, ip);
        ws.isUpgraded = true;
      } else if (msg.event === 'ClientLogin') {
        PlayerPool.addPlayer(ws, msg.data.token);
        ws.isUpgraded = true;
      }
    } catch (e) {
      Logger.log(e);
    }
  });

  // Check for unresponsive connections and close them
  ws.isAlive = true;
  ws.on('pong', heartbeat);

  // Listen for errors
  ws.on('error', error => Logger.log(`error: ${error}`));
});

function heartbeat() {
  this.isAlive = true;
}

setInterval(() => {
  wss.clients.forEach((ws) => {
    if (ws.isAlive === false) {
      return ws.terminate();
    }
    ws.isAlive = false;
    ws.ping('', false, true);
  });
}, 30000);

Logger.log('LobbyServer listening');
