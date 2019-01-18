/**
 * @file
 * Displays all factories and general controls
 */
import { colorShade } from '../Graphics/Color';
import { drawMinionLimit } from './Elements';
import { pad } from '../Util';
import Box from '../Game/Models/Box';
import Cache from '../Graphics/Cache';
import Config from '../Config/Config';
import Event from '../Event';
import Game from '../Game/Game';
import GameServer from '../Network/GameServer';
import Graphics from '../Graphics/Graphics';
import Ping from '../Network/Ping';
import Session from '../Session/Session';
import Sound from '../Sound';

let factories;

let canvas;
let context;
Event.on('cache clear', () => {
  canvas = Cache.create('StandardBarCache', 1920, 184);
  context = canvas.getContext('2d');
});

const emote = new Box(1504, 928, 128, 128);
const help = new Box(1632, 928, 128, 128);
const settings = new Box(1760, 928, 128, 128);

emote.on('mouseup', () => {
  Sound.play('select');
  const availableEmotes = [];
  const account = Session.getAccount();
  const emotes = Object.values(Config.get('emotes'));
  // Loop over emotes, adding unlocked ones
  emotes.forEach((e) => {
    // If we have unlocked this emote, add it to the select wheel
    if (account.unlocks.indexOf(e.id) >= 0) {
      availableEmotes.push({ id: e.id, title: e.emote });
    }
  });
  Screens.show('select-overlay', 1, availableEmotes, (id) => {
    if (Game.status === 2) GameServer.emit('PlayEmoteGamePhase', { id });
    if (Game.status === 3) GameServer.emit('PlayEmoteFinalizePhase', { id });
  });
});

settings.on('mouseup', () => {
  Sound.play('select');
  Screens.show('settings-ingame', 1);
});

help.on('mouseup', () => {
  Sound.play('select');
  Screens.show('game-guide', 1);
});

const StandardBar = {

  draw(ctx, x, y) {
    ctx.drawImage(canvas, x, y);
  },

  toggleListeners(state) {
    this.toggleFactoryListeners(state);
    // And for the three buttons.
    emote.listening = state;
    help.listening = state;
    settings.listening = state;
  },

  toggleFactoryListeners(state) {
    factories.forEach((item, index) => {
      factories[index].box.listening = state;
    });
  },

  update: () => {
    context.clearRect(0, 0, 1920, 184);

    context.drawImage(Cache.get('StandardBar'), 0, 0);

    // Draw Factory Interfaces on top
    factories.forEach((factory) => {
      // TODO why the coords here, not via this?!
      factory.draw(context, factory.box.x, 30);
    });

    context.drawImage(Cache.get('gameTimeAndPing'), 1216, 92);
    context.drawImage(Cache.get('emoteBtn'), 1504, emote.activeY(30));
    context.drawImage(Cache.get('helpBtn'), 1632, help.activeY(30));
    context.drawImage(Cache.get('settingsBtn'), 1760, settings.activeY(30));

    drawMinionLimit(context);
  },

};

Event.on('draft phase', () => {
  emote.listening = false;
  help.listening = false;
  settings.listening = false;
});

Event.on('game phase', () => {
  // eslint-disable-next-line prefer-destructuring
  factories = Game.state.players[Game.playerIndex].factories;
  emote.listening = true;
  help.listening = true;
  settings.listening = true;
  StandardBar.toggleFactoryListeners(false);
});

Event.on('match colors ready', () => {
  // Cache the interface background.
  Cache.set('StandardBar', 1920, 184, (ctx) => {
    ctx.userInterfaceBackground();
    ctx.font = '22px "Exo 2"';
    ctx.strokeFillText('TIME', 1264, 76, 'center', '#666');
    ctx.strokeFillText('FPS', 1344, 76, 'center', '#666');
    ctx.strokeFillText('PING', 1424, 76, 'center', '#666');
  });
  // Create the cache for time and ping.
  Cache.create('gameTimeAndPing', 384, 64);
  // Cache the help button.
  if (Cache.get('helpBtn') === false) {
    Cache.set('helpBtn', 128, 128, (ctx) => {
      ctx.customRectagon(10, 10, 108, 108, 2);
      ctx.offStrokeFill();
      ctx.drawIcon('help', colorShade('second', Game.playerIndex));
    });
  }
  // Cache the emote button.
  if (Cache.get('emoteBtn') === false) {
    Cache.set('emoteBtn', 128, 128, (ctx) => {
      ctx.customRectagon(10, 10, 108, 108, 2);
      ctx.offStrokeFill();
      ctx.drawIcon('emote', colorShade('second', Game.playerIndex));
    });
  }
  // Cache settings button.
  if (Cache.get('settingsBtn') === false) {
    Cache.set('settingsBtn', 128, 128, (ctx) => {
      ctx.customRectagon(10, 10, 108, 108, 2);
      ctx.offStrokeFill();
      ctx.drawIcon('settings', colorShade('second', Game.playerIndex));
    });
  }
});

Event.on('game over', () => {
  emote.listening = false;
  help.listening = false;
  settings.listening = false;
});

Event.on(['game phase', 'slow network update'], () => {
  Cache.update('gameTimeAndPing', (ctx) => {
    ctx.clearRect(0, 0, 256, 32);
    ctx.font = '24px "Exo 2"';
    let textColor;
    // Draw game time
    ctx.strokeFillText(
      `${
        pad(Math.floor(Game.state.time / 60), 2, '0')
      }:${
        pad(Math.floor(Game.state.time % 60), 2, '0')
      }`,
      48,
      16
    );
    // Draw FPS
    const { fps } = Graphics;
    if (fps > 40) {
      textColor = '#69B61C';
    } else if (fps > 20) {
      textColor = '#FF6700';
    } else {
      textColor = '#CF1020';
    }
    ctx.strokeFillText(`${fps}`, 130, 16, 'center', textColor);
    // Draw Ping
    const ping = Ping.get();
    if (ping < 100) {
      textColor = '#69B61C';
    } else if (ping < 300) {
      textColor = '#FF6700';
    } else {
      textColor = '#CF1020';
    }
    ctx.strokeFillText(`${ping}ms`, 208, 16, 'center', textColor);
  });
});

export default StandardBar;
