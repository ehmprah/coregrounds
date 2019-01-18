/**
 * @file
 * Manage the modpage selection
 *
 * TODO maybe add flyout animation
 */
import TWEEN from 'tween.js';
import { formatSeconds } from '../../Util';
import Box from '../../Game/Models/Box';
import Cache from '../Cache';
import Event from '../../Event';
import Game from '../../Game/Game';
import GameServer from '../../Network/GameServer';
import Session from '../../Session/Session';
import Sound from '../../Sound';

let pages;
const btn = new Box(768, 1208, 384, 128);

// Add click handler for the btn
btn.listening = false;
btn.on('mousedown', () => {
  Sound.play('select');
  const available = [];
  Object.keys(pages).forEach((pid) => {
    available.push({
      id: pid,
      title: pages[pid].title,
    });
  });
  Screens.show('select-overlay', 1, available, (id) => {
    GameServer.emit('ModPageSelection', { pid: id });
  });
});

export default {
  draw(context) {
    // TODO show game time!
    const { pid } = Game.state.players[Game.playerIndex];
    // We always show the label and the title of the currently selected page
    context.strokeFillText('MODIFICATION PAGE', 960, 512, 'center', '#666');
    if (pid) {
      context.strokeFillText(pages[pid].title, 960, 576, 'center', '#dbdbdb');
    } else {
      context.strokeFillText('none selected', 960, 576, 'center', '#dbdbdb');
    }
    // If the player has more than one, we also show the select button
    context.font = '28px "Exo 2"';
    if (Object.keys(pages).length > 1) {
      context.drawImage(Cache.get('SelectModPageButton'), btn.x, btn.y);
      context.strokeFillText(formatSeconds(Game.state.time), 960, btn.y + 160);
    } else {
      context.strokeFillText(formatSeconds(Game.state.time), 960, 840 + 160);
    }
  },
};

Event.on('game started', () => {
  ({ pages } = Session.getAccount().modifications);
});

Event.on('cache clear', () => {
  Cache.create('SelectModPageButton', 384, 128);
});

Event.on('match colors ready', () => {
  Cache.update('SelectModPageButton', (ctx) => {
    ctx.customRectagon(10, 10, 364, 108, 4);
    ctx.defaultStrokeFill();
    ctx.font = '36px "Exo 2"';
    ctx.strokeFillText('SELECT PAGE', 192, 64, 'center', '#dbdbdb');
  });
});

Event.on('picks finished', () => {
  // Only show the select button if we have more than one page
  if (Object.keys(pages).length > 1) {
    btn.listening = true;
    // Reset box position
    btn.x = 768;
    btn.y = 1208;
    // Move it in
    new TWEEN.Tween(btn).to({ y: 840 }, 300).easing(TWEEN.Easing.Quartic.In).start();
  }
});

Event.on('game phase', () => {
  btn.listening = false;
  Screens.hide(1);
});
