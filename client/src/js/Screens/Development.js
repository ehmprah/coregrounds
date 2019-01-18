/**
 * @file
 * Adds a gate for updated terms of service
 */
import { request } from '../API';
import Icons from './Elements/Icons';

Screens.add('development', () => {
  Screens.update('development', `
    <throbber>LOADING DEVELOPMENT STATUS</throbber>
  `);

  // Send request to webserver
  request('app/dev', false, (response) => {
    Screens.update('development', `
      <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content-text">
        <h3>Development Status</h3>
        <div class="development-status" style="margin-bottom: 2em;">${response.dev}</div>
        <p>Coregrounds will constantly be improved and developed and needs your help to do so! Join us
        on Discord and Reddit and let us know your opinion about the game, ideas or problems you
        have! Our Discord also is a great place to get quick help from fellow players or find
        someone to play against.</p>
        <div>
          <button class="box wide" style="margin: 0.5em 0;" onclick="window.open('https://discord.gg/aDukSU7');">
            Report bug via Discord
          </button>
          <button class="box wide" style="margin: 0.5em 0;" onclick="window.open('https://www.reddit.com/r/coregrounds/submit?selftext=true&title=[BUG]%20Bug%20Report%20Title&text=Your%20detailed%20bug%20report');">
            Report bug via Reddit
          </button>
          <button class="box wide" style="margin: 0.5em 0;" onclick="location.href='mailto:support@coregrounds.com';">
            Report bug via mail
          </button>
        </div>
      </div>
    `);
  });
});
