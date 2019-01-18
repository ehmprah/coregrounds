/**
 * @file
 * The ingame user interface, all aspects of it
 *
 * TODO while pressing ctrl (17), a factory will upgrade the lowest stat
 * TODO while pressing alt (18), a single click on a minion factory should immediately spawn
 */
import { drawGlobalTarget } from './Elements';
import { pointIsOnTile, pointsDistance, arrayRandom } from '../Util';
import { color } from '../Graphics/Color';
import Box from '../Game/Models/Box';
import Event from '../Event';
import FactoryBar from './FactoryBar';
import Game from '../Game/Game';
import Input from '../Input';
import Keyboard from '../Keyboard';
import GameServer from '../Network/GameServer';
import Overlays from '../Graphics/Overlays/Overlays';
import Sound from '../Sound';
import StandardBar from './StandardBar';
import Visuals from '../Graphics/Visuals/Visuals';
import InterfaceTips from '../Interface/InterfaceTips';
import MinionFactory from '../Game/Models/Factories/MinionFactory';

let selected = -1;
let dragging = -1;
let current = StandardBar;

Event.on('cache clear', () => {
  selected = -1;
  dragging = -1;
  current = StandardBar;
});

// The UI also handles clicks on the map
const map = new Box(0, 0, 1920, 896, true);
// We need another box on the UI area to properly process dragEnd events
const ui = new Box(0, 896, 1920, 184, true);

const UserInterface = {

  isDragging() {
    return dragging >= 0;
  },

  selectedIndex() {
    return selected;
  },

  selectedFactory() {
    return Game.state.players[Game.playerIndex].factories[selected];
  },
  // We store a reference to the factories in the game state for performance

  draw(context) {
    current.update();
    current.draw(context, 0, 896);

    InterfaceTips.draw(context);

    // Show the name of the currently selected factory or ability
    if (selected >= 0) {
      drawFactoryName(context, Game.state.players[Game.playerIndex].factories[selected]);
    }
    if (dragging >= 0) {
      drawFactoryName(context, Game.state.players[Game.playerIndex].factories[dragging]);
    }

    // Draw dragger and cancel area
    if (dragging >= 0) {
      Game.state.players[Game.playerIndex].factories[dragging].drawDragger(context);

      // Display a helper hue over the ui area, signaling cancellation
      context.fillStyle = 'rgba(150, 0, 0, 0.1)';
      context.fillRect(0, 896, 1920, 184);
    }

    if (Game.state.players[Game.playerIndex].globalTarget) {
      drawGlobalTarget(context);
    }
  },

  dragStart(index) {
    dragging = index;
    current.toggleListeners(false);
  },

  select(n) {
    deselect();
    selected = n;
    Game.state.players[Game.playerIndex].factories[n].selected = true;
    if (n > 0 && n < 6) {
      FactoryBar.setFactory(Game.state.players[Game.playerIndex].factories[n]);
      UserInterface.turnTo(FactoryBar);
    }
  },

  reset() {
    if (Game.status > 1) {
      Overlays.hide();
      deselect();
      if (selected > 0 && selected < 6) {
        UserInterface.turnTo(StandardBar);
      }
      dragging = -1;
      selected = -1;
      current.toggleListeners(true);
    }
  },

  endAllDrags() {
    if (dragging >= 0) {
      current.toggleListeners(true);
      UserInterface.reset();
      return false;
    }
  },

  turnTo(next) {
    if (current === next) return;
    // Swap listeners
    current.toggleListeners(false);
    next.toggleListeners(true);
    // Swap the bars
    current = next;
  },
};

function drawFactoryName(ctx, factory) {
  const w = ctx.measureText(factory.name).width;
  ctx.beginPath();
  ctx.moveTo(0, 900);
  ctx.lineTo(0 + w + 60, 900);
  ctx.lineTo(0 + w + 20, 850);
  ctx.lineTo(0, 850);
  ctx.lineTo(0, 900);
  ctx.fillStyle = '#000';
  ctx.fill();
  ctx.fillStyle = color();
  ctx.font = '30px "Exo 2"';
  ctx.textAlign = 'left';
  ctx.textBaseline = 'middle';
  ctx.fillText(factory.name, 20, 875);
}

function deselect() {
  Game.state.players[Game.playerIndex].factories.forEach((factory) => {
    factory.selected = false;
  });
}

function mapClick() {
  // We only run this handler when we have a running game
  if (Game.status === 2) {
    // Finish dragging
    if (dragging >= 0) {
      Game.state.players[Game.playerIndex].factories[dragging].dragEnd();
      dragging = -1;
      current.toggleListeners(true);
      return;
    }

    // Execute the selected factory's action
    if (selected >= 0) {
      Game.state.players[Game.playerIndex].factories[selected].action();
      return;
    }

    // Check if we have hit a new potential globalTarget
    globalTargetSelection();
  }
}

/**
 * Helper function checking for a new globalTarget
 */
function globalTargetSelection() {
  const entities = Object.values(Game.state.entities);
  const oldTarget = Game.state.players[Game.playerIndex].globalTarget;
  const pos = Input.position();
  const tile = Input.tile();
  let distance = 0;
  let min = 96; // You have to click within a few px of a minion to select it
  let newTarget = -1;

  // First we check for towers and minions
  entities.forEach((entity) => {
    // Only select enemy units
    if (entity.player_id !== Game.playerIndex) {
      // If we have a tower on the clicked tile, this is a no-brainer
      if (entity.type === 'Tower' || entity.type === 'Core') {
        if (pointIsOnTile(entity.x, entity.y, tile)) {
          newTarget = entity.entity_id;
        }
      }
      // Otherwise we try to find the minion closest to the click position
      if (entity.type === 'Minion') {
        distance = pointsDistance(pos.x, pos.y, entity.x + 64, entity.y + 64);
        if (distance < min) {
          min = distance;
          newTarget = entity.entity_id;
        }
      }
    }
  });

  // If we haven't found a new target so far, we do a second pass for walls
  if (newTarget === -1) {
    entities.forEach((entity) => {
      // Only select enemy units
      if (entity.player_id !== Game.playerIndex) {
        // If we have a tower on the clicked tile, this is a no-brainer
        if (entity.type === 'Wall') {
          if (pointIsOnTile(entity.x, entity.y, tile)) {
            newTarget = entity.entity_id;
          }
        }
      }
    });
  }

  // If the selected unit already is the globalTarget, turn it off
  if (newTarget === oldTarget) {
    newTarget = 0;
  }

  // Transmit changes if we have any
  if (newTarget >= 0) {
    GameServer.emit('ChangeGlobalTarget', { guid: parseInt(newTarget, 10) });
  }
}

function keyboardToggleFactory(n, evt) {
  // We only do this if we have a running game
  if (Game.status === 2 && Game.state.warmup === 0) {
    const factory = Game.state.players[Game.playerIndex].factories[n];
    if (n > 1) {
      if (evt.ctrlKey || evt.shiftKey) {
        if (factory instanceof MinionFactory) return factory.action();
      } else if (evt.altKey) {
        return factory.upgradeLowest();
      }
    }
    if (selected !== n) UserInterface.reset();
    // Select the chosen factory
    factory.select();
  }
}

function keyboardToggleNextFactory() {
  // We only do this if we have a running game
  if (Game.status === 2 && Game.state.warmup === 0) {
    const { factories } = Game.state.players[Game.playerIndex];
    const s = selected < 0 || selected >= 5
      ? 0
      : selected;
    for (let n = 1; (s + n) <= 5; n++) {
      if (factories[(s + n)].cooldown_percentage === 0) {
        factories[(s + n)].select();
        break;
      }
    }
  }
}

function endlessSpawn() {
  // We only do this if we have a running game
  if (Game.status === 2 && Game.state.warmup === 0) {
    // Get indexes of all spawn candidates
    const candidates = [];
    for (let f = 1, factory; f < 6; f++) {
      factory = Game.state.players[Game.playerIndex].factories[f];
      if (factory.stacks_current > 0 && factory.cooldown_seconds === 0) {
        candidates.push(factory);
      }
    }
    if (candidates.length) {
      const chosen = arrayRandom(candidates);
      Visuals.unitDrop(chosen.eid, { x: chosen.box.x + 64, y: 990 });
      GameServer.emit('ActivateFactory', {
        index: chosen.index,
        position: 0,
      });
      Sound.play('spawn_minion');
    }
  }
}

// Add keyboard shortcuts
Keyboard.on(32, endlessSpawn);

// Tab key
Keyboard.on(9, keyboardToggleNextFactory);

// Factories
Keyboard.on(49, keyboardToggleFactory, 0);
Keyboard.on(50, keyboardToggleFactory, 1);
Keyboard.on(51, keyboardToggleFactory, 2);
Keyboard.on(52, keyboardToggleFactory, 3);
Keyboard.on(53, keyboardToggleFactory, 4);
Keyboard.on(54, keyboardToggleFactory, 5);

// Abilities
Keyboard.on(81, keyboardToggleFactory, 6);
Keyboard.on(87, keyboardToggleFactory, 7);
Keyboard.on(69, keyboardToggleFactory, 8);

// Add click handlers
map.on('mouseup', mapClick);
ui.on('mouseup', UserInterface.endAllDrags);

Event.on('warmup finished', () => {
  current.toggleListeners(true);
});

Event.on('finalize phase', () => {
  UserInterface.reset();
  if (typeof current.toggleFactoryListeners === 'function') {
    current.toggleFactoryListeners(false);
  }
  if (Game.state.players && Game.state.players[Game.playerIndex]) {
    Game.state.players[Game.playerIndex].factories.forEach((factory) => {
      if (factory.entityBox) factory.entityBox.listening = false;
    });
  }
});

export default UserInterface;
