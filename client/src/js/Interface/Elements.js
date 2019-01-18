/**
 * @file
 * Draw UI elements in the foreground
 */
import Game from '../Game/Game';
import Event from '../Event';
import Cache from '../Graphics/Cache';
import { color } from '../Graphics/Color';
import { getLinear } from '../Graphics/Counters';

export function drawMinionLimit(context) {
  // TODO add caching
  const { players } = Game.state;

  context.lineCap = 'square';
  context.lineWidth = 3;

  let left = 0;
  let right = 1;
  if (Game.playerIndex === 1) {
    left = 1;
    right = 0;
  }

  if (players[left].minion_limit) {
    context.beginPath();
    context.moveTo(0, 5.5);
    context.lineTo(956 * players[left].minion_limit, 5.5);
    context.strokeStyle = color(1, left);
    context.stroke();
  }

  if (players[right].minion_limit) {
    context.beginPath();
    context.moveTo(1920, 5.5);
    context.lineTo(1912 - (956 * players[right].minion_limit), 5.5);
    context.strokeStyle = color(1, right);
    context.stroke();
  }

  context.beginPath();
  context.moveTo(956, 5);
  context.lineTo(956, 6);
  context.lineWidth = 4;
  context.strokeStyle = '#666';
  context.stroke();
}

export function drawGlobalTarget(context) {
  const guid = Game.state.players[Game.playerIndex].globalTarget;
  const entity = Game.state.entities[guid];

  if (entity !== undefined) {
    context.save();
    context.translate(entity.x + 64, entity.y + 64);
    context.rotate(((getLinear() % 10000) / 10000) * 2 * Math.PI);
    context.drawImage(Cache.get('globalTarget'), -64, -64);
    context.restore();
  }
}

Event.on('game started', () => {
  Cache.set('globalTarget', 128, 128, (context) => {
    context.globalAlpha = 0.5;
    context.drawIcon('globalTarget', color(1, Game.playerIndex), 116, 128);
  });
});
