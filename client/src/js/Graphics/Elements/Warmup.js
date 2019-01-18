/**
 * @file
 * Displays warmup overlay
 */
import Game from '../../Game/Game';
import { color } from '../Color';

export default {
  draw(context) {
    context.fillStyle = color(0.1);
    context.fillRect(0, 256, 1920, 384);
    context.font = '60px "Exo 2"';
    context.strokeFillText('GAME STARTING IN', 960, 360);
    context.font = '80px "Exo 2"';
    context.strokeFillText((Game.state.warmup / 10).toFixed(1), 960, 500);
  },
};
