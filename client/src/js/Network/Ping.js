/**
 * @file
 * Ping and connection handling
 */
import Event from '../Event';
import GameServer from './GameServer';
import LobbyServer from './LobbyServer';
import Config from '../Config/Config';
import Queue from '../Screens/Elements/Queue';

let timer;
let sent;
let last;
let current = 0;

export default {

  get() {
    return current;
  },

  update() {
    last = Date.now();
    // Once we receive the pong message, we know the ping
    current = last - sent;
  },

};

Event.on('joined lobby', () => {
  last = Date.now();
  timer = setInterval(pingLobbyServer, 5000);
});

Event.on('game started', () => {
  last = Date.now();
  timer = setInterval(pingGameServer, 5000);
});

Event.on(['game finished', 'game over', 'left lobby'], () => {
  clearInterval(timer);
});

function pingLobbyServer() {
  if (!LobbyServer.isConnected() || last < Date.now() - 10000) {
    if (Queue.active()) Queue.stop();
    Event.fire('game over');
    Screens.show('connection-lost');
    clearInterval(timer);
  }
  // Keep track of the time
  sent = Date.now();
  // And send a ping message
  LobbyServer.emit('ClientPing');
}

function pingGameServer() {
  if (Config.User.get('disable-reconnect')) return;
  if (!GameServer.isConnected() || last < Date.now() - 10000) {
    return gameConnectionLost();
  }
  // Keep track of the time
  sent = Date.now();
  // And send a ping message
  GameServer.emit('ping');
}

function gameConnectionLost() {
  Event.fire('game over');
  Screens.show('connection-lost');
}
