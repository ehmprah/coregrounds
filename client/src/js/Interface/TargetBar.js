/**
 * @file
 * Displays target controls for a single factory
 */
import { color } from '../Graphics/Color';
import { drawMinionLimit } from './Elements';
import Box from '../Game/Models/Box';
import Cache from '../Graphics/Cache';
import Event from '../Event';
import GameServer from '../Network/GameServer';
import UserInterface from './UserInterface';

let factory;
let canvas;
let context;

const buttons = [
  { key: 'targetTypes', name: 'core', box: new Box(176, 928, 176, 112) },
  { key: 'targetTypes', name: 'walls', box: new Box(368, 928, 176, 112) },
  { key: 'targetTypes', name: 'towers', box: new Box(560, 928, 176, 112) },
  { key: 'targetTypes', name: 'minions', box: new Box(752, 928, 176, 112) },
  { key: 'target', name: 'low', box: new Box(976, 928, 176, 112) },
  { key: 'target', name: 'high', box: new Box(1168, 928, 176, 112) },
  { key: 'target', name: 'first', box: new Box(1360, 928, 176, 112) },
  { key: 'target', name: 'random', box: new Box(1552, 928, 176, 112) },
];

const icon = new Box(32, 928, 128, 112);
const close = new Box(1760, 928, 128, 112);

buttons[0].box.on('mouseup', () => {
  setTarget('type', 'core');
});

buttons[1].box.on('mouseup', () => {
  setTarget('type', 'walls');
});

buttons[2].box.on('mouseup', () => {
  setTarget('type', 'towers');
});

buttons[3].box.on('mouseup', () => {
  setTarget('type', 'minions');
});

buttons[4].box.on('mouseup', () => {
  setTarget('target', 'low');
});

buttons[5].box.on('mouseup', () => {
  setTarget('target', 'high');
});

buttons[6].box.on('mouseup', () => {
  setTarget('target', 'first');
});

buttons[7].box.on('mouseup', () => {
  setTarget('target', 'random');
});

close.on('mouseup', () => {
  UserInterface.reset();
});

icon.on('mouseup', () => {
  UserInterface.reset();
});

const TargetBar = {

  setFactory(selected) {
    factory = selected;
  },

  draw(ctx, x, y) {
    ctx.drawImage(canvas, x, y);
  },

  update() {
    // We have to use a clearRect here if we have partly transparent UIs
    context.clearRect(0, 0, 1920, 184);

    // Draw TargetBar background
    context.drawImage(Cache.get('TargetBar'), 0, 0);

    // Draw the factory
    factory.draw(context, 32, icon.activeY(30));

    buttons.forEach((btn) => {
      context.drawImage(
        Cache.get(`targetBtn_${btn.name}_${factory[btn.key][btn.name]}`),
        btn.box.x - 10, // Spacing in the cached button
        btn.box.activeY(30)
      );
    });

    context.drawImage(Cache.get('closeBtn'), 1760, close.activeY(30));

    drawMinionLimit(context);
  },

  toggleListeners(state) {
    buttons.forEach((btn) => {
      btn.box.listening = state;
    });

    icon.listening = state;
    close.listening = state;
  },

};

TargetBar.toggleListeners(false);

export default TargetBar;

function setTarget(type, button) {
  let types;
  let mode;

  // Set the actual change
  if (type === 'type') {
    factory.targetTypes[button] = 1 - factory.targetTypes[button];
  }

  if (type === 'target') {
    Object.keys(factory.target).forEach((target) => {
      factory.target[target] = target === button ? 1 : 0;
    });
  }

  types = factory.targetTypes.minions;
  types = factory.targetTypes.towers | (types << 1);
  types = factory.targetTypes.walls | (types << 1);
  types = factory.targetTypes.core | (types << 1);

  if (factory.target.low) mode = 1;
  if (factory.target.high) mode = 2;
  if (factory.target.first) mode = 3;
  if (factory.target.random) mode = 4;

  GameServer.emit('ChangeTargetPreference', {
    index: UserInterface.selectedFactory().index,
    targetPreference: types | (mode << 15),
  });
}

Event.on('game started', () => {
  // Create the cache for the target bar
  canvas = Cache.create('TargetBarCache', 1920, 184);
  context = canvas.getContext('2d');
  // And its background
  Cache.set('TargetBar', 1920, 184, (ctx) => {
    ctx.userInterfaceBackground();
  });
  // Precache all buttons in all states
  const btns = [
    { type: 'type', name: 'core' },
    { type: 'type', name: 'walls' },
    { type: 'type', name: 'towers' },
    { type: 'type', name: 'minions' },
    { type: 'target', name: 'low' },
    { type: 'target', name: 'high' },
    { type: 'target', name: 'first' },
    { type: 'target', name: 'random' },
  ];
  [0, 1].forEach((state) => {
    btns.forEach((btn) => {
      Cache.set(
        `targetBtn_${btn.name}_${state}`,
        200,
        128,
        cacheTargetPreferenceButton,
        btn.name.toUpperCase(),
        state,
        btn.type
      );
    });
  });
});

function cacheTargetPreferenceButton(ctx, name, state, type) {
  // Draw the button
  ctx.customRectagon(10, 10, 176, 108, 2);
  ctx.defaultStrokeFill();
  ctx.font = '24px "Exo 2"';
  ctx.strokeFillText(name, 98, 44);
  // Draw the state
  if (type === 'target') {
    if (state) {
      ctx.strokeFillText('TARGET', 98, 84, 'center', color());
    }
  } else if (state) {
    ctx.strokeFillText('ON', 98, 84, 'center', color());
  } else {
    ctx.strokeFillText('OFF', 98, 84, 'center', '#B1233A');
  }
}
