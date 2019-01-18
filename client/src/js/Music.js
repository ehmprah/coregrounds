/**
 * @file
 * Handles the adaptive music
 */
import Howler from 'howler';
import Event from './Event';
import Config from './Config/Config';
import Game from './Game/Game';
import { arrayRandom } from './Util';

const stack = [];
let timeout;
let heat = 0;
let volume = 1;
let next;
let current;
let bucket = 'ambient';

const buckets = {
  draft: [
    new Howler.Howl({ src: ['music/picks.ogg'], volume: 0 }),
  ],
  ambient: [
    new Howler.Howl({ src: ['music/A1.mp3'], volume: 0 }),
    new Howler.Howl({ src: ['music/A2.mp3'], volume: 0 }),
    new Howler.Howl({ src: ['music/A3.mp3'], volume: 0 }),
    new Howler.Howl({ src: ['music/A4.mp3'], volume: 0 }),
    new Howler.Howl({ src: ['music/A5.mp3'], volume: 0 }),
    new Howler.Howl({ src: ['music/U1.mp3'], volume: 0 }),
    new Howler.Howl({ src: ['music/U2.mp3'], volume: 0 }),
  ],
  heroic: [
    new Howler.Howl({ src: ['music/H1.mp3'], volume: 0 }),
    new Howler.Howl({ src: ['music/H2.mp3'], volume: 0 }),
    new Howler.Howl({ src: ['music/H3.mp3'], volume: 0 }),
    new Howler.Howl({ src: ['music/H4.mp3'], volume: 0 }),
    new Howler.Howl({ src: ['music/H5.mp3'], volume: 0 }),
  ],
  lowTension: [
    new Howler.Howl({ src: ['music/B1.mp3'], volume: 0 }),
    new Howler.Howl({ src: ['music/B2.mp3'], volume: 0 }),
  ],
  highTension: [
    new Howler.Howl({ src: ['music/P2.mp3'], volume: 0 }),
    new Howler.Howl({ src: ['music/P3.mp3'], volume: 0 }),
    new Howler.Howl({ src: ['music/P4.mp3'], volume: 0 }),
    new Howler.Howl({ src: ['music/P5.mp3'], volume: 0 }),
  ],
  connectors: [
    new Howler.Howl({ src: ['music/P1.mp3'], volume: 0 }),
    new Howler.Howl({ src: ['music/P1B.mp3'], volume: 0 }),
    new Howler.Howl({ src: ['music/P1D.mp3'], volume: 0 }),
  ],
};

function setVolume() {
  Object.keys(buckets).forEach((name) => {
    buckets[name].forEach((track) => {
      track.volume(volume);
    });
  });
}

function nextTrack() {
  // If we have tracks scheduled, play those first
  if (stack.length) {
    next = stack.shift();
  } else {
    // Otherwise we'll determine the current bucket by the heat
    getBucket();
    // And pick a track at random from the chosen bucket
    next = arrayRandom(buckets[bucket]);
  }

  if (current) {
    // Fade out the currently playing track
    current.fade(volume, 0, Config.FADE_DURATION);
    // If we have only one track in the bucket or the same one got randomly
    // selected, we will just replay it and directly schedule the next
    if (current === next) {
      return setTimeout(() => {
        current.stop();
        current.play();
        current.fade(0, volume, Config.FADE_DURATION);
        timeout = setTimeout(nextTrack, (next.duration() * 1000) - Config.FADE_DURATION);
      }, Config.FADE_DURATION);
    }
  }

  // Swap references
  current = next;

  // Start the next one and start fading it in
  next.stop();
  next.play();
  next.fade(0, volume, Config.FADE_DURATION);

  // We get a new file shortly before this one stops playing.
  timeout = setTimeout(nextTrack, (next.duration() * 1000) - Config.FADE_DURATION);
}

function getBucket() {
  // Without an active game, we default to the ambient bucket
  if (Game.status === 0) {
    bucket = 'ambient';
  }

  // During the draft phase, we always choose the draft bucket
  if (Game.status === 1) {
    bucket = 'draft';
  }

  // During the game phase, we choose buckets according to heat
  if (Game.status === 2) {
    if (heat === 0) {
      bucket = 'ambient';
    }
    if (heat > 0) {
      bucket = 'heroic';
    }
    if (heat > 10) {
      bucket = 'lowTension';
    }
    if (heat > 25) {
      bucket = 'highTension';
    }
  }
}

Event.on('every second', () => {
  // if (current) console.log(`Playing ${current._src} (Bucket: ${bucket}, Heat: ${heat})`);
  if (heat > 0) heat -= 0.5;
});

Event.on('game over', () => {
  heat = 0;
});

Event.on('draft phase', () => {
  clearTimeout(timeout);
  nextTrack();
});

Event.on('game phase', () => {
  stack.push(buckets.heroic[0]);
  clearTimeout(timeout);
  nextTrack();
});

Event.on(['app loaded', 'music volume changed'], () => {
  // Update the local volume variable
  volume = Config.User.get('music-volume', 1);
  // And update all sounds
  setVolume();
});

Event.on('after load', () => {
  // We start playing as soon as the first track is loaded
  buckets.heroic[0].once('load', nextTrack);
  // We always start with the same tracks for identity & branding reasons
  stack.push(buckets.heroic[0]);
});

Event.on(['enemy Tower killed', 'own Tower killed'], () => {
  heat += 5;
});

Event.on(['enemy Wall killed', 'own Wall killed'], () => {
  heat += 2;
});

Event.on(['enemy Minion killed', 'own Minion killed'], () => {
  heat += 1;
});
