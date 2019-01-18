/**
 * @file
 * The top layer which renders the UI and overlays
 *
 * @todo cache background properly, add shortcuts for nested vars.
 */
import Picks from '../Elements/Picks';
import FogOfWar from '../Elements/FogOfWar';
import Emote from '../Elements/Emote';
import EndOfGame from '../Elements/EndOfGame';
import UnitTooltip from '../Elements/UnitTooltip';
import Game from '../../Game/Game';
import Overlays from '../../Graphics/Overlays/Overlays';
import Cache from '../../Graphics/Cache';
import UserInterface from '../../Interface/UserInterface';
import Visuals from '../Visuals/Visuals';
import Particles from '../Particles/Particles';
import Event from '../../Event';
import Warmup from '../Elements/Warmup';

let canvas;
let context;
Event.on('cache clear', () => {
  canvas = Cache.create('Foreground', 1920, 1080);
  context = canvas.getContext('2d');
});

export default {

  draw(ctx) {
    ctx.drawImage(canvas, 0, 0);
  },

  update() {
    context.clearRect(0, 0, 1920, 1080);

    // Draw pick and ban phase
    if (Game.status === 1) {
      Picks.draw(context);
      // TODO move this and dependency to Picks
      UnitTooltip.draw(context);
    }

    // Draw UI
    if (Game.status >= 2) {
      // First we draw visuals and particles
      Visuals.draw(context);
      Particles.draw(context);
      // Then the fog of war
      FogOfWar.draw(context);
      // And then all of the interface
      Overlays.draw(context);
      UserInterface.draw(context);

      if (Game.state.warmup) {
        Warmup.draw(context);
      }

      if (Game.status === 3) {
        EndOfGame.draw(context);
      }
    }

    Emote.draw(context);
  },

};
