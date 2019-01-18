/**
 * @file FactoryBar.js
 * Displays controls for a single selected factory
 *
 * @todo ADD update duration and death timer between upgrade btns, with an icon?!
 */
import Event from '../Event';
import Box from '../Game/Models/Box';
import Cache from '../Graphics/Cache';
import GameServer from '../Network/GameServer';
import UserInterface from './UserInterface';
import { color, colorShade } from '../Graphics/Color';
import TargetBar from './TargetBar';
import { drawMinionLimit } from './Elements';
import Sound from '../Sound';
import Game from '../Game/Game';

let canvas;
let context;
let factory;

const icon = new Box(32, 928, 128, 128);
const target = new Box(1504, 928, 128, 128);
const help = new Box(1632, 928, 128, 128);
const close = new Box(1760, 928, 128, 128);

const upgradeBtns = [];
upgradeBtns.push(new Box(176, 936, 380, 112));
upgradeBtns.push(new Box(572, 936, 380, 112));
upgradeBtns.push(new Box(968, 936, 380, 112));

upgradeBtns[0].on('mouseup', () => {
  GameServer.emit('UpgradeFactory', { index: factory.index, upgrade: 0 });
  UserInterface.reset();
  Sound.play('select');
  return false;
});

upgradeBtns[1].on('mouseup', () => {
  GameServer.emit('UpgradeFactory', { index: factory.index, upgrade: 1 });
  UserInterface.reset();
  Sound.play('select');
  return false;
});

upgradeBtns[2].on('mouseup', () => {
  GameServer.emit('UpgradeFactory', { index: factory.index, upgrade: 2 });
  UserInterface.reset();
  Sound.play('select');
  return false;
});

icon.on('mouseup', () => {
  UserInterface.reset();
});

help.on('mouseup', () => {
  Screens.show('game-guide-detail', 1, factory.eid);
});

close.on('mouseup', () => {
  UserInterface.reset();
});

target.on('mouseup', () => {
  if (factory.hasTargetPreference) {
    TargetBar.setFactory(UserInterface.selectedFactory());
    UserInterface.turnTo(TargetBar);
    // Return false to stop propagation
    return false;
  }
});

Event.on('cache clear', () => {
  canvas = Cache.create('FactoryBarCache', 1920, 184);
  context = canvas.getContext('2d');
});

const FactoryBar = {

  setFactory(selected) {
    factory = selected;
    target.listening = factory.hasTargetPreference;
  },

  draw(ctx, x, y) {
    ctx.drawImage(canvas, x, y);
  },

  update() {
    // We have to use a clearRect here if we have partly transparent UIs
    context.clearRect(0, 0, 1920, 184);

    // Draw factory UI background
    context.drawImage(Cache.get('FactoryBar'), 0, 0);

    // Draw the factory
    factory.draw(context, 32, icon.activeY(30));

    // Determine current maximum possible upgrade
    const max = getMaximumUpgradeLevel();

    // Draw update buttons
    upgradeBtns.forEach((button, index) => {
      const lvl = factory[`upgrade_${index + 1}`];
      const x = button.x - 10; // we need to subtract the spacing
      const y = button.activeY(30);

      context.drawImage(Cache.get('upgradeBtn'), x, y);
      context.drawImage(Cache.get(`upgrade_${lvl}_${max}`), x + 84, y + 94);

      context.font = '28px "Exo 2"';
      if (factory.upgradableStatNames) {
        context.strokeFillText(factory.upgradableStatNames[index], x + 200, y + 66);
      }
    });

    context.font = '24px "Exo 2"';
    context.strokeFillText(getUpgradeDuration(), 1432, 92);

    if (factory.hasTargetPreference) {
      context.drawImage(Cache.get('targetBtn'), target.x, target.activeY(30));
    }

    context.drawImage(Cache.get('helpBtn'), help.x, help.activeY(30));
    context.drawImage(Cache.get('closeBtn'), close.x, close.activeY(30));

    drawMinionLimit(context);
  },

  toggleListeners(state) {
    upgradeBtns[0].listening = state;
    upgradeBtns[1].listening = state;
    upgradeBtns[2].listening = state;
    close.listening = state;
    icon.listening = state;
    help.listening = state;
    target.listening = state;
  },
};

FactoryBar.toggleListeners(false);

function getMaximumUpgradeLevel() {
  let maxUpgrade = 4;
  if (factory.upgrade_1 < maxUpgrade) maxUpgrade = factory.upgrade_1;
  if (factory.upgrade_2 < maxUpgrade) maxUpgrade = factory.upgrade_2;
  if (factory.upgrade_3 < maxUpgrade) maxUpgrade = factory.upgrade_3;
  return Math.min(4, maxUpgrade + 2);
}

function getUpgradeDuration() {
  const sum = factory.upgrade_1 + factory.upgrade_2 + factory.upgrade_3;
  if (sum < 12) {
    return `${(1 + sum) * 5}s`;
  }
  return 'MAX';
}

Event.on('match colors ready', () => {
  Cache.set('FactoryBar', 1920, 184, (ctx) => {
    ctx.userInterfaceBackground();
    ctx.font = '20px "Exo 2"';
    ctx.strokeFillText('UPGRADE', 1432, 60, 'center', '#666');
    ctx.strokeFillText('DURATION', 1432, 124, 'center', '#666');
  });

  Cache.set('targetBtn', 128, 128, (ctx) => {
    ctx.customRectagon(10, 10, 108, 108, 2);
    ctx.offStrokeFill();
    ctx.drawIcon('target', colorShade('second', Game.playerIndex));
  });

  Cache.set('closeBtn', 128, 128, (ctx) => {
    ctx.customRectagon(10, 10, 108, 108, 2);
    ctx.offStrokeFill();
    ctx.drawIcon('close', colorShade('second', Game.playerIndex));
  });

  Cache.set('upgradeBtn', 400, 128, (ctx) => {
    ctx.customRectagon(10, 10, 380, 108, 2);
    ctx.defaultStrokeFill();
    ctx.font = '24px "Exo 2"';
    ctx.strokeFillText('UPGRADE', 200, 32, 'center', '#666');
  });

  // Cache all level indicator options
  let lvl;
  let max;
  for (lvl = 0; lvl <= 4; lvl++) {
    for (max = 2; max <= 4; max++) {
      if (lvl < 4 || max === 4) {
        Cache.set(`upgrade_${lvl}_${max}`, 256, 16, cacheUpgradeLevelIndicator, lvl, max);
      }
    }
  }
});

function cacheUpgradeLevelIndicator(ctx, current, max) {
  let offset = 2;
  ctx.lineWidth = 2;
  ctx.strokeStyle = '#000';
  for (let l = 0; l <= 4; l++) {
    ctx.beginPath();
    ctx.rect(offset, 2, 40, 9);
    if (current >= l) {
      ctx.fillStyle = color();
    } else if (l <= max) {
      ctx.fillStyle = '#333';
    } else {
      ctx.fillStyle = '#111';
    }
    ctx.fill();
    ctx.stroke();
    offset += 48;
  }
}

export default FactoryBar;
