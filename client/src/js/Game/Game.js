/**
 * @file
 * Holds the game state
 */
import TWEEN from 'tween.js';
import Event from '../Event';
import Sound from '../Sound';
/* globals nw */

const Game = {
  finished: false,
  matchId: 0,
  status: 0,
  playerIndex: 0,
  teamId: 0,
  winningTeamId: 0,
  state: {},

  running() {
    return Game.status > 0;
  },

  phase() {
    return Game.status;
  },
};

Event.on(['app loaded', 'game over'], reset);

function reset() {
  // Turn off input boxes
  if (Game.state.players && Game.state.players[Game.playerIndex]) {
    Game.state.players[Game.playerIndex].factories.forEach((factory) => {
      if (factory.box.off !== undefined) {
        factory.box.off('mouseup');
      }
      factory.box.dragStart = false;
      if (factory.entityBox) {
        if (factory.entityBox.off !== undefined) {
          factory.entityBox.off('mouseup');
        }
        factory.entityBox.dragStart = false;
      }
    });
  }
  Game.finished = false;
  Game.matchId = 0;
  Game.status = 0;
  Game.playerIndex = 0;
  Game.teamId = 0;
  Game.winningTeamId = 0;
  Game.state = {
    match_type: 0,
    warmup: 0,
    warmup_finished: false,
    time: 0,
    draft: 0,
    activePlayerIndex: 0,
    players: [],
    entities: {},
  };

  // Remove all tweens that might be active still
  TWEEN.removeAll();

  Event.fire('game reset');
}

Event.on('game started', () => {
  Sound.play('upgrade_complete');
  document.title = 'IN GAME - COREGROUNDS';
  if (nw) nw.Window.get().focus();
});

Event.on('game over', () => {
  document.title = 'COREGROUNDS';
});

export default Game;
