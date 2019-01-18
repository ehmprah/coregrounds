/**
 * @file
 * Template for queue timer element
 */
import Event from '../../Event';
import Config from '../../Config/Config';
import Sound from '../../Sound';
import { adoptHTML } from '../../Util';
import Icons from '../../Screens/Elements/Icons';
import Queue from '../../Screens/Elements/Queue';
import Session from '../../Session/Session';

Event.on('after load', () => {
  adoptHTML(`
    <div id="menu-tint" onclick="hideMenu();"></div>
    <div id="menu">
      <button class="close-menu" onclick="hideMenu()">${Icons.get('close')}</button>
      <div class="links"></div>
    </div>
  `);
});

window.showMenu = function showMenu() {
  const account = Session.getAccount();

  let html = '';

  if (Queue.active()) {
    html += `
      <button disabled>
        <div class="icon">${Icons.get('wait')}</div>
        <div>IN QUEUE</div>
      </button>
    `;
  } else {
    html += `
      <button onclick="Screens.show('${account.tutorial ? 'play' : 'tutorial'}'); hideMenu();">
        <div class="icon">${Icons.get('play')}</div>
        <div>PLAY</div>
      </button>
    `;
  }

  html += `
    <button onclick="Screens.show('modifications'); hideMenu();">
      <div class="icon">${Icons.get('modify')}</div>
      <div>MODIFICATIONS</div>
    </button>
  `;

  html += `
    <button onclick="Screens.show('unlock-towers'); hideMenu();">
      <div class="icon">${Icons.get('unlock')}</div>
      <div>UNLOCK</div>
    </button>
  `;

  html += `
    <button onclick="Screens.show('settings-customize'); hideMenu();">
      <div class="icon">${Icons.get('settings')}</div>
      <div>SETTINGS</div>
    </button>
  `;

  html += `<div class="menu-spacer"></div>`;

  html += `
    <button onclick="Screens.show('game-guide'); hideMenu();">
      <div class="icon">${Icons.get('help')}</div><div>GAME GUIDE</div>
    </button>
  `;

  html += `
    <button onclick="Screens.show('traininggrounds'); hideMenu();">
      <div class="icon">${Icons.get('traininggrounds')}</div><div>TRAINING GROUNDS</div>
    </button>
  `;

  html += `<div class="menu-spacer"></div>`;

  html += `
    <button onclick="Screens.show('matches'); hideMenu();">
      <div class="icon">${Icons.get('matchHistory')}</div>
      <div>MATCH HISTORY</div>
    </button>
  `;

  html += `
    <button onclick="Screens.show('scoregrounds-bot'); hideMenu();">
      <div class="icon">${Icons.get('scoregrounds')}</div>
      <div>SCOREGROUNDS</div>
    </button>
  `;

  html += `<div class="menu-spacer"></div>`;

  html += `
    <button onclick="Screens.show('changelog-${Config.VERSION}'); hideMenu();">
      <div class="icon">${Icons.get('changelog')}</div>
      <div>VERSION ${Config.VERSION}</div>
    </button>
  `;

  html += `
    <button onclick="Screens.show('about'); hideMenu();">
      <div class="icon">${Icons.get('about')}</div>
      <div>ABOUT</div>
    </button>
  `;

  html += `
    <button onclick="Screens.show('quit'); hideMenu();">
      <div class="icon">${Icons.get('quit')}</div>
      <div>QUIT</div>
    </button>
  `;

  // Render HTML
  $('#menu .links').innerHTML = html;

  // Add click sound for buttons
  $$('#menu .links button:not(.silent)').on('click', () => {
    Sound.play('select');
  });

  // Show the menu
  $('#menu').classList.add('active');
  $('#menu-tint').classList.add('active');
};

window.hideMenu = function hideMenu() {
  $('#menu').classList.remove('active');
  $('#menu-tint').classList.remove('active');
};
