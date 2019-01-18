/**
 * @file
 * Displays game winner
 */
import Cache from '../Cache';
import Game from '../../Game/Game';
import Config from '../../Config/Config';
import Event from '../../Event';
import Session from '../../Session/Session';
import Box from '../../Game/Models/Box';
import Sound from '../../Sound';
import GameServer from '../../Network/GameServer';
import Analytics from '../../Analytics';

const btn = new Box(704, 496, 512, 128);
btn.on('mouseup', () => {
  Sound.play('select');
  // Save the match id before resetting the match
  const matchId = Game.matchId;
  // Disconnect the game server
  GameServer.disconnect();
  // Reset the match before changing the screens
  Event.fire('game over');
  // Change the screen and reload the player object
  if (matchId > 0) {
    Screens.show('match', 0, matchId);
  } else {
    Screens.show('matches');
  }
  Session.reload();
  return false;
});

export default {
  draw(context) {
    context.drawImage(Cache.get('EndOfGame'), 0, 256);
    if (!Game.matchId) {
      context.drawImage(Cache.get('BtnContinue'), btn.x, btn.activeY());
    } else {
      context.drawImage(Cache.get('BtnContinueReady'), btn.x, btn.activeY());
    }
  },
};

Event.on('cache clear', () => {
  btn.listening = false;
  Cache.create('EndOfGame', 1920, 384);
  Cache.create('BtnContinue', 512, 128);
  Cache.create('BtnContinueReady', 512, 128);
});

Event.on('match colors ready', () => {
  Cache.update('BtnContinue', (context) => {
    context.customRectagon(10, 10, 492, 108, 2);
    context.defaultStrokeFill();
    context.font = '28px "Exo 2"';
    context.strokeFillText('CONTINUE', 256, 85, 'center');
    context.font = '28px "Exo 2"';
    context.strokeFillText('WAITING FOR MATCH HISTORY...', 256, 45, 'center', '#666');
  });
  Cache.update('BtnContinueReady', (context) => {
    context.customRectagon(10, 10, 492, 108, 2);
    context.defaultStrokeFill();
    context.font = '28px "Exo 2"';
    context.strokeFillText('CONTINUE', 256, 85, 'center');
    context.font = '28px "Exo 2"';
    context.strokeFillText('MATCH HISTORY READY!', 256, 45, 'center', '#666');
  });
});

Event.on('finalize phase', () => {
  // Now we listen to the continue button
  btn.listening = true;
  // And we update the end of game screen
  const opponent = Game.state.players[(1 - Game.playerIndex)];
  // Send analytics event
  if (Game.winningTeamId === 0) {
    Analytics.event('Game', 'draw');
  } else if (opponent.teamId === Game.winningTeamId) {
    Analytics.event('Game', 'lost game');
  } else {
    Analytics.event('Game', 'won game');
  }
  // Update cache
  Cache.update('EndOfGame', (context) => {
    context.fillStyle = 'rgba(0, 0, 0, 0.95)';
    context.fillRect(0, 0, 1920, 384);
    context.font = '40px "Exo 2"';
    if (Game.winningTeamId === 0) {
      context.strokeFillText('DRAW AGAINST', 960, 50);
    } else if (opponent.teamId === Game.winningTeamId) {
      context.strokeFillText('YOU HAVE BEEN DEFEATED BY', 960, 50, 'center', 'rgb(120, 30, 30)');
    } else {
      context.strokeFillText('YOU HAVE DEFEATED', 960, 50, 'center', 'rgb(30, 120, 30)');
    }
    const xOffset = 960 - 64 - (context.measureText(opponent.name).width / 2);
    const icon = Config.query('playerIcons', { id: opponent.icon });
    const image = new Image();
    image.src = `player-icons/${icon.file}`;
    image.onload = () => {
      context.drawImage(image, xOffset, 112, 96, 96);
    };
    context.font = '50px "Exo 2"';
    context.strokeFillText(opponent.name, xOffset + 128, 160, 'left');
  });
});
