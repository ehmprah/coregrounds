/**
 * @file
 * Counts streaks and adds motivational messages and voice overs
 *
 * TODO add voice overs!
 */
import Event from '../Event';
import Config from '../Config/Config';
import Messages from '../Screens/Elements/Messages';

const streaks = {
  towers: {
    last: 0,
    streak: -1,
    messages: [
      'Tower Kill',
      'Double Tower Kill',
      'Triple Tower Kill',
      'Quadra Tower Kill',
      'Penta Tower Kill',
    ],
  },
};

Event.on('enemy Tower killed', () => {
  const now = Date.now();
  if (now <= streaks.towers.last + Config.TOWER_KILLSTREAK_DELAY) {
    streaks.towers.streak += 1;
    streaks.towers.last = now;
  } else {
    // Display first message
    streaks.towers.streak = 0;
    streaks.towers.last = now;
  }
  // Display message according to current streak
  Messages.show(streaks.towers.messages[streaks.towers.streak]);
});

Event.on('own Tower killed', () => {
  Messages.show('Tower lost');
});
