/**
 * @file
 * Manage and play sound effects
 *
 * TODO reduce volume of ERROR sound
 * TODO increase volume of TOGGLE_INTERFACE sound
 */
import Howler from 'howler';
import Event from './Event';
import Config from './Config/Config';

const sounds = {};
let volume = 0.75;
let explosions = 0;

const Sound = {

  play(key, delay = 0) {
    // Respect the delay
    if (delay) {
      setTimeout(() => {
        Sound.play(key);
      }, delay);
      return;
    }

    if (sounds[key] !== undefined) {
      if (key !== 'explosion') {
        sounds[key].play();
      } else if (explosions <= 3) {
        explosions += 1;
        sounds[key].play();
      }
    } else {
      throw new Error(`Trying to play undefined sound "${key}"`);
    }
  },

  stop(sound) {
    if (sounds[sound] !== undefined) {
      sounds[sound].stop();
    }
  },

  stopAll() {
    Object.keys(sounds).forEach((sound) => {
      sounds[sound].pause();
    });
  },
};

Event.on(['app loaded', 'sound volume changed'], () => {
  // Update the local volume variable
  volume = Config.User.get('sound-volume', 1);
  // And update all sounds
  setVolume();
});

Event.on('after load', () => {
  // Add all sound FX
  [
    { key: 'buff', file: 'sfx/buff.wav' },
    { key: 'build_tower', file: 'sfx/build_tower.wav' },
    { key: 'build_wall', file: 'sfx/build_wall.wav' },
    { key: 'select', file: 'sfx/click.wav' },
    { key: 'countdown', file: 'sfx/countdown.wav' },
    { key: 'debuff', file: 'sfx/debuff.wav' },
    { key: 'error', file: 'sfx/error.wav' },
    { key: 'explosion', file: 'sfx/explosion.wav' },
    { key: 'full_stack', file: 'sfx/full_stack.wav' },
    { key: 'heal', file: 'sfx/heal.wav' },
    { key: 'ionstrike_onset', file: 'sfx/ionstrike_onset.wav' },
    { key: 'ionstrike_strike', file: 'sfx/ionstrike_strike.wav' },
    { key: 'move_tower', file: 'sfx/move_tower.wav' },
    { key: 'notification', file: 'sfx/notification.wav' },
    { key: 'shield', file: 'sfx/shield.wav' },
    { key: 'shot_aoe', file: 'sfx/shot_aoe.wav' },
    { key: 'shot_bouncing', file: 'sfx/shot_bouncing.wav' },
    { key: 'shot_fast', file: 'sfx/shot_fast.wav' },
    { key: 'shot_lightning', file: 'sfx/shot_lightning.wav' },
    { key: 'shot_medium', file: 'sfx/shot_medium.wav' },
    { key: 'shot_railgun_heavy', file: 'sfx/shot_railgun_heavy.wav' },
    { key: 'shot_railgun_light', file: 'sfx/shot_railgun_light.wav' },
    { key: 'shot_slow', file: 'sfx/shot_slow.wav' },
    { key: 'loss', file: 'sfx/lose_match.wav' },
    { key: 'spawn_minion', file: 'sfx/spawn_minion.wav' },
    { key: 'toggle_interface', file: 'sfx/toggle_interface.wav' },
    { key: 'upgrade_complete', file: 'sfx/upgrade_complete.wav' },
    { key: 'win', file: 'sfx/win_match.wav' },
  ].forEach((sound) => {
    sounds[sound.key] = new Howler.Howl({
      src: [sound.file],
      volume,
    });
  });

  // Add extra callback to explosion sound to decrease the limit counter
  sounds.explosion.on('end', () => {
    explosions -= 1;
  });
});

Event.on('screen rendered', () => {
  $$('screen button:not(.silent)').on('click', () => {
    console.log(this);
    Sound.play('select');
  });
  $$('screen .clickable').on('click', () => {
    Sound.play('select');
  });
  // TODO BUG multiple renders on layer 1 register this callback multiple times.
});

function setVolume() {
  Object.keys(sounds).forEach((sound) => {
    sounds[sound].volume(volume);
  });
}

export default Sound;
