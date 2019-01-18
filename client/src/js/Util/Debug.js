import { color } from '../Graphics/Color';
import Config from '../Config/Config';

export function drawStats(context, entity) {
  context.font = '11px Arial';
  let y = entity.y + 58 + (entity.stats.length * 6);
  entity.stats.forEach((stat) => {
    context.strokeFillText(stat, entity.x + 64, y);
    y -= 12;
  });
}

export function drawArea(context, entity) {
  let item;
  if (entity.type === 'Wall') {
    item = Config.query('walls', { id: entity.eid });
  }
  if (entity.type === 'Core') {
    item = Config.query('cores', { id: entity.eid });
  }
  if (entity.type === 'Minion') {
    item = Config.query('minions', { id: entity.eid });
  }
  if (entity.type === 'Tower') {
    item = Config.query('towers', { id: entity.eid });
  }
  if (item) {
    context.fillStyle = color(0.5, entity.player_index);
    context.beginPath();
    context.arc(entity.x + 64, entity.y + 64, item.area.circle.radius * 128, 0, 2 * Math.PI);
    context.closePath();
    context.fill();
  }
}
