/**
 * @file
 * Displays the pick phase
 *
 * TODO add fly in animations at both beginning and end of draft
 * TODO alignPicks should have different animation times as per distance
 * TODO mabye scale up the selected pick in the middle a bit
 * TODO maybe it a wrap around wheel, repeating picks if necessary
 * TODO add drag and drop to the picks, like a slider
 */
import TWEEN from 'tween.js';
import { color, colorShade } from '../Color';
import { formatSeconds } from '../../Util';
import Config from '../../Config/Config';
import Box from '../../Game/Models/Box';
import Cache from '../Cache';
import UnitCache from '../UnitCache';
import Event from '../../Event';
import Game from '../../Game/Game';
import Keyboard from '../../Keyboard';
import GameServer from '../../Network/GameServer';
import Session from '../../Session/Session';
import Sound from '../../Sound';
import Pick from './Pick';
import ModificationSelection from './ModificationSelection';

let selected = 0;
let selectedIndex = -1;
const picks = [];
const panel = { x: 0, y: 1540 };

let freePicks = [];

let canvas;
let ctx;
Event.on('cache clear', () => {
  selected = 0;
  canvas = Cache.create('PickPanelCache', 1920, 512);
  ctx = canvas.getContext('2d');
  Cache.create('DraftBackground', 1920, 1080);
  Cache.create('banBtn', 384, 128);
  Cache.create('pickBtn', 384, 128);
  Cache.create('nextBtn', 128, 128);
  Cache.create('prevBtn', 128, 128);
  Cache.create('emoteBtn', 128, 128);
  Cache.create('settingsBtn', 128, 128);
  Cache.create('FreePickFlag', 128, 20);
});

const prevBtn = new Box(640, 1208, 128, 128);
const pickBtn = new Box(768, 1208, 384, 128);
const nextBtn = new Box(1152, 1208, 128, 128);
const emote = new Box(1472, 840, 128, 128);
const help = new Box(1600, 840, 128, 128);
const settings = new Box(1728, 840, 128, 128);

// Add click handlers
prevBtn.on('mousedown', cycle, false);
nextBtn.on('mousedown', cycle, true);
pickBtn.on('mousedown', finalizePick);

emote.on('mouseup', () => {
  Sound.play('select');
  const available = [];
  const account = Session.getAccount();
  // Loop over emotes, adding unlocked ones
  Config.get('emotes').forEach((e) => {
    if (account.unlocks.indexOf(e.id) >= 0) available.push({ id: e.id, title: e.emote });
  });
  Screens.show('select-overlay', 1, available, (id) => {
    GameServer.emit('PlayEmotePickPhase', { id });
  });
});

help.on('mouseup', () => {
  Sound.play('select');
  Screens.show('game-guide', 1);
});

settings.on('mouseup', () => {
  Sound.play('select');
  Screens.show('settings-ingame', 1);
});

// Add keyboard shortcuts
Keyboard.on(37, cycle, false);
Keyboard.on(39, cycle, true);
Keyboard.on(13, finalizePick);

const Picks = {

  updateFreePicks(data) {
    freePicks = data.slice();
  },

  pick(id, playerIndex, index) {
    const pick = getPickById(id);
    pick.updateStatus('picked', playerIndex, index);
  },

  ban(id, playerIndex, index) {
    const pick = getPickById(id);
    pick.updateStatus('banned', playerIndex, index);
  },

  draw(context) {
    // Draw cached background
    context.drawImage(Cache.get('DraftBackground'), 0, 0);

    // Draw texts
    drawText(context);

    // Draw all picks
    picks.forEach((pick) => {
      pick.draw(context);
      if (pick.isPickable() && freePicks.indexOf(pick.id) > -1) {
        context.drawImage(Cache.get('FreePickFlag'), pick.box.x, pick.box.y + 128);
      }
    });

    // Draw the pick panel
    if (Game.state.draft < 5) {
      // Draw cached panel
      context.drawImage(canvas, panel.x, panel.y);
      // Draw buttons
      context.drawImage(Cache.get('prevBtn'), prevBtn.x, prevBtn.activeY());
      context.drawImage(Cache.get('nextBtn'), nextBtn.x, nextBtn.activeY());
      // Add highlight shape
      context.beginPath();
      context.moveTo(960, 804);
      context.lineTo(980, 824);
      context.lineTo(940, 824);
      context.lineTo(960, 804);
      if (Game.activePlayerIndex === Game.playerIndex) {
        context.fillStyle = color(1);
      } else {
        context.fillStyle = '#666';
      }
      context.fill();
      // Draw CTA
      if (Game.activePlayerIndex === Game.playerIndex) {
        if (Game.state.draft > 2) {
          context.drawImage(Cache.get('pickBtn'), pickBtn.x, pickBtn.activeY());
        } else {
          context.drawImage(Cache.get('banBtn'), pickBtn.x, pickBtn.activeY());
        }
      } else {
        context.font = '36px "Exo 2"';
        context.strokeFillText('WAIT', pickBtn.x + 192, pickBtn.y + 54, 'center');
      }
      // Show time left
      context.font = '28px "Exo 2"';
      context.strokeFillText(formatSeconds(Game.state.time), 960, pickBtn.y + 160);
    } else {
      ModificationSelection.draw(context);
    }

    // Draw extra buttons
    context.drawImage(Cache.get('emoteBtn'), emote.x, emote.activeY());
    context.drawImage(Cache.get('helpBtn'), help.x, help.activeY());
    context.drawImage(Cache.get('settingsBtn'), settings.x, settings.activeY());
  },

  select(id) {
    const pick = getPickById(id);
    if (pick) {
      // Deselect the old one
      if (selected) {
        const previous = getPickById(selected);
        previous.selected = false;
      }
      // Select the new one
      pick.selected = true;
      selected = id;
      const pickable = picks.filter(p => p.isPickable());
      selectedIndex = pickable.findIndex(p => p.id === selected);

      // Play sound
      Sound.play('select');
      // Set new selected pick
      cachePanel(pick);
      // Move picks to their appropriate position
      alignPicks();
    } else {
      throw new Error('WTF?!');
    }
  },
};

Event.on('draft subphase changed', () => {
  selected = 0;
  selectedIndex = -1;
});

Event.on('picks finished', () => {
  prevBtn.listening = false;
  nextBtn.listening = false;
  pickBtn.listening = false;
});

Event.on('match colors ready', () => {
  Cache.update('DraftBackground', (context) => {
    // Draw slots for the picks
    for (let p = 0; p < 2; p++) {
      // Draw the ban slots like the bans with alpha
      context.strokeStyle = '#000';
      context.lineWidth = 2;
      // Draw rectangle for factory ban
      context.beginPath();
      context.rect(68 + (p * 1664), 132, 120, 120);
      context.fillStyle = '#111';
      context.fill();
      context.fillStyle = 'hsla(0, 0%, 60%, 0.15)';
      context.fill();
      context.stroke();
      // Draw circle for ability ban
      context.beginPath();
      context.arc(128 + (p * 1664), 320, 56, 0, 2 * Math.PI);
      context.fillStyle = '#111';
      context.fill();
      context.fillStyle = 'hsla(0, 0%, 60%, 0.15)';
      context.fill();
      context.stroke();
      // Now switch to colors for the rest
      for (let i = 1; i <= 5; i++) {
        // Draw rectangles for factory slots
        context.beginPath();
        context.rect(132 + (i * 128) + (p * 768), 132, 120, 120);
        context.fillStyle = '#111';
        context.fill();
        context.fillStyle = color(0.15, p);
        context.fill();
        context.stroke();
      }
      for (let i = 1; i <= 3; i++) {
        context.beginPath();
        context.arc(320 + (i * 128) + (p * 768), 320, 56, 0, 2 * Math.PI);
        context.fillStyle = '#111';
        context.fill();
        context.fillStyle = color(0.15, p);
        context.fill();
        context.stroke();
      }
    }
    // Label the ban slots
    context.font = '32px "Exo 2"';
    context.strokeFillText('BANS', 128, 96, 'center', 'hsla(0, 0%, 60%, 1)');
    context.strokeFillText('PICKS', 576, 96, 'center', color(1, 0));
    context.strokeFillText('BANS', 1792, 96, 'center', 'hsla(0, 0%, 60%, 1)');
    context.strokeFillText('PICKS', 1344, 96, 'center', color(1, 1));
  });

  Cache.update('banBtn', (context) => {
    context.customRectagon(10, 10, 364, 108, 2);
    context.defaultStrokeFill();
    context.font = '36px "Exo 2"';
    context.strokeFillText('BAN', 192, 64, 'center');
  });

  Cache.update('pickBtn', (context) => {
    context.customRectagon(10, 10, 364, 108, 2);
    context.defaultStrokeFill();
    context.font = '36px "Exo 2"';
    context.strokeFillText('PICK', 192, 64, 'center');
  });

  Cache.update('nextBtn', (context) => {
    context.customRectagon(10, 10, 108, 108, 2);
    context.defaultStrokeFill();
    context.drawIcon('next', color(0.5));
  });

  Cache.update('prevBtn', (context) => {
    context.customRectagon(10, 10, 108, 108, 2);
    context.defaultStrokeFill();
    context.drawIcon('prev', color(0.5));
  });

  Cache.update('helpBtn', (context) => {
    context.customRectagon(10, 10, 108, 108, 2);
    context.offStrokeFill();
    context.drawIcon('help', colorShade('second', Game.playerIndex));
  });

  Cache.update('emoteBtn', (context) => {
    context.customRectagon(10, 10, 108, 108, 2);
    context.offStrokeFill();
    context.drawIcon('emote', colorShade('second', Game.playerIndex));
  });

  Cache.update('settingsBtn', (context) => {
    context.customRectagon(10, 10, 108, 108, 2);
    context.offStrokeFill();
    context.drawIcon('settings', colorShade('second', Game.playerIndex));
  });

  Cache.update('FreePickFlag', (context) => {
    context.font = '16px "Exo 2"';
    context.strokeFillText('FREE', 64, 10, 'center');
  });
});

Event.on('draft phase', () => {
  // Activate buttons
  prevBtn.listening = true;
  nextBtn.listening = true;
  pickBtn.listening = true;
  emote.listening = true;
  help.listening = true;
  settings.listening = true;
  // Animate the panel (base location 640 / 896)
  new TWEEN.Tween(panel).to({ y: 432 }, 300).easing(TWEEN.Easing.Quartic.In).start();
  new TWEEN.Tween(nextBtn).to({ y: 840 }, 300).easing(TWEEN.Easing.Quartic.In).start();
  new TWEEN.Tween(prevBtn).to({ y: 840 }, 300).easing(TWEEN.Easing.Quartic.In).start();
  new TWEEN.Tween(pickBtn).to({ y: 840 }, 300).easing(TWEEN.Easing.Quartic.In).start();
});

Event.on('config ready', () => {
  if (!picks.length) {
    Config.get('factories').forEach((item) => {
      if (item.pickable) picks.push(new Pick(item));
    });
    Config.get('abilities').forEach((item) => {
      if (item.pickable) picks.push(new Pick(item));
    });
  }
});

Event.on('before draft phase update', () => {
  picks.forEach((pick) => {
    pick.playerIndex = Game.playerIndex;
  });
});

Event.on('game phase', () => {
  picks.forEach(pick => pick.reset());
  // And we turn off the event listeners
  prevBtn.listening = false;
  nextBtn.listening = false;
  pickBtn.listening = false;
  emote.listening = false;
  help.listening = false;
  settings.listening = false;
});

Event.on('updated available picks', () => {
  Sound.play('notification');
  const pickable = Game.state.players[Game.playerIndex].pickable;
  picks.filter(pick => pick.isNotPickedOrBanned()).forEach((pick) => {
    if (pickable.indexOf(pick.id) >= 0 && pickFitsPhase(pick)) {
      pick.setPickable();
      pick.deselect();
    } else {
      pick.setOff();
    }
  });

  if (!selected) {
    return selectCenterPick();
  }

  const pick = getPickById(selected);
  if (!pick.isPickable()) {
    return autoNextPick();
  }
  pick.select();

  alignPicks();
});

function pickFitsPhase(pick) {
  // We can't properly display the next phase's picks without proper separation of the pickables
  const draft = Game.state.draft;
  if ((draft === 1 || draft === 3) && pick.type !== 'Ability') {
    return true;
  } else if ((draft === 2 || draft === 4) && pick.type === 'Ability') {
    return true;
  }
  return false;
}

function alignPicks() {
  // And align picks according to the selected one
  const pickable = picks.filter(pick => pick.isPickable());
  const index = pickable.findIndex(pick => pick.id === selected);
  pickable.forEach((pick, pickIndex) => {
    // Calculate offset from center
    const offset = pickIndex - index;
    // Calculate position
    const position = 896 + (offset * 128);
    // Move pick to the position
    pick.tween.to({ x: position, y: 640 }, 300).easing(TWEEN.Easing.Quartic.Out).start();
  });
}

function cycle(right) {
  // We only do this if we have a game in draft phase
  if (Game.status === 1) {
    selectNextPick(right);
  }
}

function finalizePick() {
  // We only do this if we have a game in draft phase
  if (Game.status === 1) {
    // And we only do it while it's our turn
    if (Game.activePlayerIndex === Game.playerIndex) {
      // By default, the action is ban and the target is factories
      let action = 'Pick';
      let target = 0;
      // If we're picking, update the action
      if (Game.state.draft === 1 || Game.state.draft === 2) action = 'Ban';
      // If we're picking abilities, update the target
      if (Game.state.draft === 2 || Game.state.draft === 4) target = 1;
      // Format and send the message
      GameServer.emit(action, {
        id: selected,
        target,
      });
      Sound.play('select');
    }
  }
}

function cachePanel(pick) {
  ctx.clearRect(0, 0, 1920, 512);
  // Show the unit and it's description
  const cached = UnitCache.get(pick.id, pick.playerIndex, pick.type, 'interface');
  setTimeout(() => {
    ctx.drawImage(cached, 384, 0);
  }, 0);
  ctx.font = '36px "Exo 2"';
  ctx.strokeFillText(pick.type.toUpperCase(), 544, 28, 'left');
  ctx.strokeFillText(pick.name.toUpperCase(), 682, 28, 'left', color(1, pick.playerIndex));
  ctx.font = '28px "Exo 2"';
  ctx.strokeFillText(pick.description, 544, 80, 'left');
  ctx.strokeFillText(`Difficulty: ${pick.difficulty} `, 544, 132, 'left', '#777');
  ctx.strokeFillText(`Role: ${pick.role} `, 800, 132, 'left', '#777');
  // Display strong and weak picks
  ctx.strokeFillText('Strong vs:', 1020, 132, 'left', '#777');
  pick.strong.forEach((counter, index) => {
    const sprite = UnitCache.get(counter.id, (1 - pick.playerIndex), counter.type, 'interface');
    setTimeout(() => {
      ctx.drawImage(sprite, 1150 + (index * 48), 110, 48, 48);
    }, 0);
  });
  ctx.strokeFillText('Weak vs:', 1320, 132, 'left', '#777');
  pick.weak.forEach((counter, index) => {
    const sprite = UnitCache.get(counter.id, (1 - pick.playerIndex), counter.type, 'interface');
    setTimeout(() => {
      ctx.drawImage(sprite, 1450 + (index * 48), 110, 48, 48);
    }, 0);
  });
}

function drawText(context) {
  // Display welcome message and timer
  context.font = '36px "Exo 2"';
  context.textAlign = 'center';
  if (Game.state.match_type === 4) {
    if (Game.state.picked < 18) {
      context.strokeFillText('RANDOM DRAFT', 960, 96);
    }
  }
  // TODO rework this into a state rather than a counter
  if (Game.state.picked === 18) {
    context.strokeFillText('GAME IS STARTING IN', 640, 96);
    context.strokeFillText((15 - Game.state.time), 1280, 96);
  }
}

function getPickById(id) {
  return picks.find(pick => pick.id === id);
}

function selectNextPick(right = true) {
  const pickable = picks.filter(pick => pick.isPickable());
  if (right) {
    if (pickable[(selectedIndex + 1)] !== undefined) {
      Picks.select(pickable[(selectedIndex + 1)].id);
    } else {
      Picks.select(pickable[0].id);
    }
  } else if (pickable[(selectedIndex - 1)] !== undefined) {
    Picks.select(pickable[(selectedIndex - 1)].id);
  } else {
    Picks.select(pickable[(pickable.length - 1)].id);
  }
}

function autoNextPick() {
  const pickable = picks.filter(pick => pick.isPickable());
  if (pickable[selectedIndex] !== undefined) {
    return Picks.select(pickable[selectedIndex].id);
  }
  if (pickable[(selectedIndex + 1)] !== undefined) {
    return Picks.select(pickable[(selectedIndex + 1)].id);
  }
  if (pickable[(selectedIndex - 1)] !== undefined) {
    return Picks.select(pickable[(selectedIndex - 1)].id);
  }
}

function selectCenterPick() {
  const pickable = picks.filter(pick => pick.isPickable());
  if (!pickable.length) return;
  const index = Math.floor(pickable.length / 2);
  Picks.select(pickable[index].id);
}

export default Picks;
