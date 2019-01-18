/**
 * @file
 * Handles connection to the LobbyServer
 */
import { logToConsole } from '../Console/Console';
import Config from '../Config/Config';
import Event from '../Event';
import GameServer from './GameServer';
import LobbySocket from './LobbySocket';
import Ping from './Ping';
import Session from '../Session/Session';

let socket;

export default {

  isConnected() {
    if (socket && socket.socket.readyState === 1) return true;
    return false;
  },

  on(key, cb) {
    socket.on(key, cb);
  },

  connect() {
    if (Config.User.get('lobby-server')) {
      socket = LobbySocket.connect(`ws://${Config.User.get('lobby-server')}`);
    } else {
      socket = LobbySocket.connect(`ws://${Config.LOBBYSERVER}`);
    }

    // Handle connection errors
    socket.on('error', (error) => {
      Screens.show('network-error');
      logToConsole(`Error: ${JSON.stringify(error)}`, 'error');
    });

    // Start a session
    socket.on('open', () => {
      socket.emit('ClientLogin', { token: Session.getToken() });
    });

    // Handle specific errors
    socket.on('ClientErrorMessage', (error) => {
      Screens.show('error', 0, error);
    });

    // Handle specific errors
    socket.on('ClientPong', () => {
      Ping.update();
    });

    // Handle a successul login
    socket.on('ClientLoginSuccessful', (response) => {
      // Overwrite the local config if we've got one
      if (response && response.config) {
        window.localStorage.setItem('config', JSON.stringify(response.config));
        Config.update(response.config);
      }
      Event.fire('network ready', 'joined lobby');
    });

    // Connect to game server
    socket.on('ClientGameServerReady', (data) => {
      terminateConnection();
      // Connect to gameserver
      GameServer.connect(data.ip, data.token);
      // Show connection screen
      Screens.show('status', 0, 'CONNECTING TO GAME SERVER');
    });
  },

  disconnect() {
    terminateConnection();
  },

  emit(key, data) {
    socket.emit(key, data);
  },
};

function terminateConnection() {
  Event.fire('left lobby');
  socket.close();
}
