/**
 * @file
 * Websocket connection handling for the game server
 */
import { tick } from './GameServer';
import Event from '../Event';
import i from './InputMessages_pb';
import o from './OutputMessages_pb';
import Ping from './Ping';
import Game from '../Game/Game';

// console.log(i);
// console.log(o);

let socket;

export default {
  connect(url, token) {
    // Create socket
    socket = new WebSocket(url);
    socket.binaryType = 'arraybuffer';
    // Attach handlers
    socket.onclose = () => {};
    socket.onerror = () => {
      Event.fire('network error');
    };
    socket.onopen = () => {
      setTimeout(() => {
        this.emit('login', { token });
      }, 250);
    };
    socket.onmessage = (event) => {
      const payload = o.Message.deserializeBinary(event.data);
      if (payload.hasMatch()) tick(payload.getMatch());
      if (payload.hasPong()) Ping.update();
      // Handle login failures
      if (payload.hasLogin()) {
        const result = payload.getLogin().getResult();
        if (result === 0) {
          if (!Game.running()) {
            Screens.show('status', 0, 'CONNECTED, WAITING FOR OPPONENT');
          }
        } else if (result === 2) {
          socket.close();
          Screens.show('error', 0, 'NO MATCH FOUND');
        } else if (result === 3) {
          socket.close();
          Screens.show('error', 0, 'MATCH TIMED OUT');
        } else if (result === 4) {
          socket.close();
          Screens.show('error', 0, 'OPPONENT FAILED TO CONNECT');
        }
      }
    };

    return socket;
  },

  isConnected() {
    if (socket && socket.readyState === 1) return true;
    return false;
  },

  close() {
    socket.close();
  },

  emit(key, data) {
    buildPayload(key, data);
  },
};

function buildPayload(key, data) {
  const message = new i.Message();

  if (key === 'ping') {
    const ping = new i.Ping();
    message.setPing(ping);
  }

  if (key === 'login') {
    const login = new i.Login();
    login.setToken(data.token);
    message.setLogin(login);
  }

  if (key === 'Ban') {
    const match = new i.Match();
    const pick = new i.Match.Pick();
    const payload = new i.Match.Pick.Ban();
    payload.setId(data.id);
    payload.setTarget(data.target);
    pick.addBans(payload, 0);
    match.setPick(pick);
    message.setMatch(match);
  }

  if (key === 'Pick') {
    const match = new i.Match();
    const pick = new i.Match.Pick();
    const payload = new i.Match.Pick.Pick();
    payload.setId(data.id);
    payload.setTarget(data.target);
    pick.addPicks(payload, 0);
    match.setPick(pick);
    message.setMatch(match);
  }

  if (key === 'ModPageSelection') {
    const match = new i.Match();
    const pick = new i.Match.Pick();
    const payload = new i.Match.Pick.ModPageSelection();
    payload.setPid(data.pid);
    pick.addModpageselections(payload, 0);
    match.setPick(pick);
    message.setMatch(match);
  }

  if (key === 'PlayEmotePickPhase') {
    const match = new i.Match();
    const pick = new i.Match.Pick();
    const payload = new i.Match.PlayEmote();
    payload.setId(data.id);
    pick.addEmoteplays(payload, 0);
    match.setPick(pick);
    message.setMatch(match);
  }

  if (key === 'PlayEmoteGamePhase') {
    const match = new i.Match();
    const game = new i.Match.Game();
    const payload = new i.Match.PlayEmote();
    payload.setId(data.id);
    game.addEmoteplays(payload, 0);
    match.setGame(game);
    message.setMatch(match);
  }

  if (key === 'PlayEmoteFinalizePhase') {
    const match = new i.Match();
    const finalize = new i.Match.Finalize();
    const payload = new i.Match.PlayEmote();
    payload.setId(data.id);
    finalize.addEmoteplays(payload, 0);
    match.setFinalize(finalize);
    message.setMatch(match);
  }

  if (key === 'ActivateAbility') {
    const match = new i.Match();
    const game = new i.Match.Game();
    const payload = new i.Match.Game.ActivateAbility();
    payload.setIndex(data.index);
    payload.setPosition(data.position);
    game.addAbilityactivations(payload, 0);
    match.setGame(game);
    message.setMatch(match);
  }

  if (key === 'ActivateFactory') {
    const match = new i.Match();
    const game = new i.Match.Game();
    const payload = new i.Match.Game.ActivateFactory();
    payload.setIndex(data.index);
    payload.setPosition(data.position);
    game.addFactoryactivations(payload, 0);
    match.setGame(game);
    message.setMatch(match);
  }

  if (key === 'UpgradeFactory') {
    const match = new i.Match();
    const game = new i.Match.Game();
    const payload = new i.Match.Game.UpgradeFactory();
    payload.setIndex(data.index);
    payload.setUpgrade(data.upgrade);
    game.addFactoryupgrades(payload, 0);
    match.setGame(game);
    message.setMatch(match);
  }

  if (key === 'ChangeTargetPreference') {
    const match = new i.Match();
    const game = new i.Match.Game();
    const payload = new i.Match.Game.ChangeTargetPreference();
    payload.setIndex(data.index);
    payload.setTargetpreference(data.targetPreference);
    game.addTargetpreferencechanges(payload, 0);
    match.setGame(game);
    message.setMatch(match);
  }

  if (key === 'ChangeGlobalTarget') {
    const match = new i.Match();
    const game = new i.Match.Game();
    const payload = new i.Match.Game.ChangeGlobalTarget();
    payload.setGuid(data.guid);
    game.addGlobaltargetchanges(payload, 0);
    match.setGame(game);
    message.setMatch(match);
  }

  if (key === 'Surrender') {
    const match = new i.Match();
    const game = new i.Match.Game();
    game.setSurrender(true);
    match.setGame(game);
    message.setMatch(match);
  }

  // Send the serialized array buffer
  socket.send(message.serializeBinary().buffer);
}
