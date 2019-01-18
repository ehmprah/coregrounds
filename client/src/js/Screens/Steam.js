/**
 * @file
 * Screens for login and account creation
 */
import { request } from '../API';
import { submitHandler, validateEmail, throttle } from '../Util/DOM';
import Config from '../Config/Config';
import Messages from '../Screens/Elements/Messages';
import Sound from '../Sound';
import Steam from '../Session/Steam';
/* global greenworks */

Screens.add('steam-init', () => {
  Screens.update('steam-init', `
    <div>
      <throbber style="margin-bottom: 1.5em">
        <h3 class="first">WELCOME TO THE COREGROUNDS</h3>
      </throbber>
      <div class="menu" style="margin: 0 auto;">
        <form id="steam-init" autocomplete="off">
          <div style="padding: 0 5px; font-size: 15px;">
            <label for="username">USERNAME</label>
            <div id="username-available" style="float: right;">&nbsp;</div>
          </div>
          <input type="text" id="username" class="margin-bottom" placeholder="Username" />
          <button type="submit" class="box" id="init-submit">LOGIN</button>
          <input type="submit" style="visibility: hidden; position: absolute;" />
        </form>
        <div style="margin: 1em 0; text-align: center;">or</div>
        <button class="box grey" onclick="Screens.show('steam-link')">LINK EXISTING ACCOUNT</button>
      </div>
    </div>
  `);

  // Pre-fill username with steam username
  $('#username').value = greenworks.getSteamId().getPersonaName();
  // And check it's availability
  checkUsername();

  // Add username check to the input field
  const throttledNameCheck = throttle(checkUsername, 100);
  $('#username').on('keyup', throttledNameCheck);

  submitHandler('init-submit', () => {
    if (!$('#username').value.length) {
      Messages.show('Please enter a username.', 'error');
      Sound.play('error');
      return false;
    }
    if ($('#username-available').classList.contains('not-available')) {
      Messages.show('Please choose an available username', 'error');
      Sound.play('error');
      return false;
    }
    return true;
  }, () => {
    Steam.init($('#username').value, () => {
      $('#init-submit').classList.remove('waiting');
    });
  });
});

Screens.add('steam-link', () => {
  Screens.update('steam-link', `
    <div>
      <throbber style="margin-bottom: 1.5em">
        <h3 class="first">LINK EXISTING ACCOUNT</h3>
      </throbber>
      <div class="menu">
        <form id="login-form" autocomplete="off">
          <input type="text" id="login-mail" class="margin-bottom" placeholder="E-Mail-Address" />
          <input type="password" id="login-password" class="margin-bottom" placeholder="Password" />
          <button type="submit" class="box" id="login-submit">Login</button>
          <input type="submit" style="visibility: hidden; position: absolute;" />
        </form>
        <button class="box grey" onclick="Screens.show('steam-init');">BACK</button>
        <div style="margin: 2em 0; text-align: center; font-size: 16px;">
          <a href="${Config.ENDPOINT}/player/password" target="_blank" style="border:none">
            FORGOT PASSWORD?
          </a>
        </div>
      </div>
    </div>
  `);

  // Attach submit handlers
  submitHandler('login-submit', () => {
    // Check if we have input in the mail field
    if (!$('#login-mail').value.length) {
      Messages.show('Please enter your email address.', 'error');
      Sound.play('error');
      return false;
    }
    // Check if we have input in the password field
    if (!$('#login-password').value.length) {
      Messages.show('Please enter your password.', 'error');
      Sound.play('error');
      return false;
    }
    // Check if we have a valid email address
    if (!validateEmail($('#login-mail').value)) {
      Messages.show('Please enter a valid email address.', 'error');
      Sound.play('error');
      return false;
    }
    return true;
  }, () => {
    // Execute login
    Steam.link($('#login-mail').value, $('#login-password').value, () => {
      $('#login-submit').classList.remove('waiting');
    });
  });
});

Screens.add('steam-error', () => {
  Screens.update('steam-error', `
    <throbber class="error color-310">
      <h3 class="first text-310">STEAM ERROR</h3>
      <p>Could not initialize Steam API. Please start Coregrounds from the Steam client.</p>
      <div class="menu" style="margin: auto">
        <button class="box grey" onclick="window.close()">Quit</a>
      </div>
    </throbber>
  `);
});

/**
 * Helper function firing username check requests
 */
let scheduled;
function checkUsername() {
  $('#username-available').innerHTML = 'CHECKING...';
  $('#username-available').className = '';
  // Clear any previously scheduled requests
  clearTimeout(scheduled);
  // Schedule a new request in 200ms
  scheduled = setTimeout(() => {
    request('app/check/name', {
      name: $('#username').value,
    }, () => {
      $('#username-available').innerHTML = 'AVAILABLE';
      $('#username-available').className = 'available';
    }, (response) => {
      $('#username-available').innerHTML = response.status;
      $('#username-available').className = 'not-available';
    });
  }, 200);
}
