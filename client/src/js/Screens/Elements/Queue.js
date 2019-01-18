/**
 * @file
 * Template for queue timer element
 */
import Event from '../../Event';
import LobbyServer from '../../Network/LobbyServer';
import Sound from '../../Sound';
import Analytics from '../../Analytics';
import { adoptHTML, formatTime } from '../../Util';
import Icons from './Icons';

let counter;
let elapsed;

const Queue = {

  start(estimate) {
    // Format the time
    document.getElementById('estimate').innerHTML = formatTime(estimate);
    document.getElementById('actual').innerHTML = '00:00';

    // Start the counter
    elapsed = 0;
    counter = setInterval(() => {
      elapsed += 1;
      document.getElementById('actual').innerHTML = formatTime(elapsed);
    }, 1000);

    // Show the counter
    $('#queue').classList.add('active');
  },

  stop() {
    // Stop the counter
    clearInterval(counter);
    // Hide the element
    $('#queue').classList.remove('active');
  },

  active() {
    return $('#queue').classList.contains('active');
  },
};

Event.on('game started', Queue.stop);

Event.on('after load', () => {
  // Create the DOM element
  adoptHTML(`
    <div id="queue">
      <div id="quit-queue">${Icons.get('close')}</div>
      <div class="timer">
        <div class="clearfix">Estimated<span id="estimate"></span></div>
        <div class="clearfix">Current<span id="actual"></span></div>
      </div>
    </div>
  `);
  // Add event listener for the cancel button
  $('#quit-queue').on('click', () => {
    Sound.play('select');
    Queue.stop();
    LobbyServer.emit('ClientQuitQueue');
    LobbyServer.disconnect();
    Analytics.event('Game', 'quit queue', 'duration', elapsed);
    if (Screens.current() === 'home') Screens.refresh();
  });
});

export default Queue;
