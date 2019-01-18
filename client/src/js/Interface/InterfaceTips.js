/**
 * @file
 * Displays UI tip bubbles
 */
import TWEEN from 'tween.js';
import Cache from '../Graphics/Cache';
import Game from '../Game/Game';
import Config from '../Config/Config';
import Event from '../Event';
import { color } from '../Graphics/Color';

let timer;
let walls = false;
let towers = false;
let minions = false;
const current = {
  key: '',
  offset: 0,
  alpha: 0,
};

Event.on('warmup finished', () => {
  timer = setInterval(() => {
    // If we're currently displaying a tip, we skip this
    if (current.key) return;
    // We abort the check once we have everything covered
    if (walls && towers && minions) return clearInterval(timer);
    // Otherwise we check for all three things
    const factories = Game.state.players[Game.playerIndex].factories;
    const props = Config.get('properties');

    // Check for built walls
    if (!walls) {
      if (factories[0].stacks_current === props.game.initial_walls) {
        current.key = 'InterfaceTipsWalls';
        current.offset = 96;
        new TWEEN.Tween(current).to({ alpha: 0.9 }, 300).easing(TWEEN.Easing.Quartic.In).start();
      } else {
        walls = true;
      }
      return;
    }

    // Check for built towers
    if (!towers) {
      factories.forEach((factory) => {
        if (factory.entity_id) towers = true;
      });
      if (!towers) {
        const factory = factories.find(f => f.entity_id !== undefined);
        if (factory) {
          current.key = 'InterfaceTipsTowers';
          current.offset = factory.box.x + 64;
          new TWEEN.Tween(current).to({ alpha: 0.9 }, 300).easing(TWEEN.Easing.Quartic.In).start();
        }
      }
      towers = true;
      return;
    }

    // Check for spawned minions
    if (!minions) {
      if (Game.state.players[Game.playerIndex].minion_limit === 0) {
        const factory = factories.find(f => f.stacks_current !== undefined && f.eid !== undefined);
        if (factory) {
          current.key = 'InterfaceTipsMinions';
          current.offset = factory.box.x + 64;
          new TWEEN.Tween(current).to({ alpha: 0.9 }, 300).easing(TWEEN.Easing.Quartic.In).start();
        }
      }
      minions = true;
    }
  }, 1000);
});

Event.on('match colors ready', () => {
  Cache.set('InterfaceTipsWalls', 512, 160, (context) => {
    cacheTip(context, 'Build walls');
  });
  Cache.set('InterfaceTipsTowers', 512, 160, (context) => {
    cacheTip(context, 'Build your tower');
  });
  Cache.set('InterfaceTipsMinions', 512, 160, (context) => {
    cacheTip(context, 'Spawn some minions');
  });
});

export default {
  draw(context) {
    if (current.key) {
      context.globalAlpha = current.alpha;
      context.drawImage(Cache.get(current.key), current.offset, 768);
      context.globalAlpha = 1;
    }
  },
  hideWallTip() {
    hide('InterfaceTipsWalls');
  },
  hideTowerTip() {
    hide('InterfaceTipsTowers');
  },
  hideMinionTip() {
    hide('InterfaceTipsMinions');
  },
};

function hide(type) {
  if (current.key === type) {
    new TWEEN.Tween(current)
      .to({ alpha: 0 }, 600)
      .easing(TWEEN.Easing.Quartic.Out)
      .onComplete(() => {
        current.key = '';
      })
      .start();
  }
}

function cacheTip(context, tip) {
  // We have to set the font first, or the text will not be measured correctly
  context.font = '36px "Exo 2"';
  context.fillStyle = color(0.3, Game.playerIndex);
  context.beginPath();
  const width = context.measureText(tip).width + 128;
  context.rect(0, 0, width, 128);
  context.moveTo(0, 128);
  context.lineTo(0, 160);
  context.lineTo(32, 128);
  context.lineTo(0, 128);
  context.fill();
  context.strokeFillText(tip, (width / 2), 64);
}
