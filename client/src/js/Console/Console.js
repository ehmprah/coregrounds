/**
 * @file Console.js
 * Template and logic for the console
 *
 * TODO add autocomplete here via tab key
 * TODO add error message cascade after first wrong command, e.g.
 *      "Close, but no cigar.", "Stop it.", "Brute force, huh?",
 *      "Ah ah ah a-ah. You don't know the magic word.", "STAHP! NOW!"
 *       after cascade: just quit the program, quitting in 3, 2, 1, bam!
 */
import Event from '../Event';
import Config from '../Config/Config';
import Game from '../Game/Game';
import Keyboard from '../Keyboard';
import { adoptHTML } from '../Util';
import ConsoleHistory from './ConsoleHistory';
import { request } from '../API';

export function logToConsole(msg, type = 'status') {
  // Get the input element
  const log = $('#console .log');
  const div = document.createElement('div');
  div.classList.add(type);
  div.innerHTML = msg;
  log.append(div);
  log.scrollTop = log.scrollHeight;
}

Event.on('app loaded', () => {
  adoptHTML(`
    <div id="console">
      <div class="log"></div>
      <form autocomplete="off">
        <span>></span><input type="text" id="console-input" />
        <input type="submit" style="visibility: hidden; position: absolute;" />
      </form>
    </div>
  `);

  // Add submit listener on the form
  const element = $('#console form');
  element.addEventListener('submit', submit, false);
});

Keyboard.on(120, () => {
  // F9 shows the console via CSS animation
  const element = $('#console');
  element.classList.toggle('active');
  // Reset the input element
  const input = $('#console-input');
  input.value = '';
  // And if we're showing the console, focus the input field
  if (element.classList.contains('active')) {
    input.focus();
  }
});

Keyboard.on(27, () => {
  // The escape key closes the console
  $('#console').classList.remove('active');
});

Keyboard.on(38, () => {
  if ($('#console').classList.contains('active')) {
    $('#console-input').value = ConsoleHistory.last();
  }
});

Keyboard.on(40, () => {
  if ($('#console').classList.contains('active')) {
    $('#console-input').value = ConsoleHistory.next();
  }
});

function submit(e) {
  // Don't submit the form as usual
  e.preventDefault();

  const element = $('#console-input');
  const input = element.value;
  let msg = 'Command not found';

  if (!input.length) return;

  // Add this input to the history
  ConsoleHistory.add(input);

  if (input === 'help') {
    msg = 'COMMANDS: ';
    msg += '<br>log state (logs game state to js console once)';
    msg += '<br>toggle reconnect (disables reconnect screen)';
    msg += '<br>toggle network logging (logs all game traffic to js console)';
    msg += '<br>toggle stats (show unit stats)';
    msg += '<br>toggle area (shows unit colliders)';
    msg += '<br>set apiserver https://domain.com/ (override api server)';
    msg += '<br>set lobbyserver hostname:port (override lobby server address and port)';
    msg += '<br>set gameserver hostname:port (override game server address and port)';
    msg += '<br>set local region (override region setting to local)';
    msg += '<br>reset servers (reset server addresses to default)';
  }

  if (input === 'log state') {
    // eslint-disable-next-line
    console.log(Game);
    msg = 'Game state logged to console.';
  }

  if (input === 'toggle network logging') {
    // Get previously saved value
    let flag = Config.User.get('network-logging');
    // Toggle that value
    flag = !flag;
    // Save the new setting
    Config.User.set('network-logging', flag);
    // And display message with the new setting
    msg = `Network logging is now turned ${flag ? 'on' : 'off'}`;
  }

  if (input === 'toggle reconnect') {
    // Get previously saved value
    let flag = Config.User.get('disable-reconnect');
    // Toggle that value
    flag = !flag;
    // Save the new setting
    Config.User.set('disable-reconnect', flag);
    // And display message with the new setting
    msg = `Reconnect ${flag ? 'disabled' : 'enabled'}`;
  }

  if (input === 'toggle stats') {
    // Get previously saved value
    let flag = Config.User.get('debug-stats');
    // Toggle that value
    flag = !flag;
    // Save the new setting
    Config.User.set('debug-stats', flag);
    // And display message with the new setting
    msg = `Debug status now turned ${flag ? 'on' : 'off'}`;
  }

  if (input === 'toggle area') {
    // Get previously saved value
    let flag = Config.User.get('debug-area');
    // Toggle that value
    flag = !flag;
    // Save the new setting
    Config.User.set('debug-area', flag);
    // And display message with the new setting
    msg = `Debug area now turned ${flag ? 'on' : 'off'}`;
  }

  if (input.substr(0, 14) === 'set gameserver') {
    // Save the new setting
    Config.User.set('game-server', input.substr(15));
    // And display message with the new setting
    msg = `Game server address set to ${input.substr(15)}`;
  }

  if (input.substr(0, 15) === 'set lobbyserver') {
    // Save the new setting
    Config.User.set('lobby-server', input.substr(16));
    // And display message with the new setting
    msg = `Lobby server address set to ${input.substr(16)}`;
  }

  if (input.substr(0, 13) === 'set apiserver') {
    // Save the new setting
    Config.User.set('api-server', input.substr(14));
    // And display message with the new setting
    msg = `API server address set to ${input.substr(14)}`;
  }

  if (input.substr(0, 16) === 'set local region') {
    // Save the new setting
    request('app/set-region', {
      region: 'local',
      crossregion: 0,
    }, () => {});
    // And display message with the new setting
    msg = `Region set to local`;
  }

  if (input === 'reset servers') {
    // Save the new setting
    Config.User.set('game-server', false);
    Config.User.set('lobby-server', false);
    // And display message with the new setting
    msg = 'Server addresses reset to default';
  }

  logToConsole(input);
  logToConsole(msg, 'clr-text');

  // Reset the input
  element.value = '';
}
