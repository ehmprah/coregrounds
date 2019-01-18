/**
 * @file
 * Screens for login and account creation
 *
 * TODO add extra checkbox "remember me on this device"
 * TODO check if we really need the quit button here. maybe add the minimize / quit btns at the top?
 */
import { request } from '../API';
import { submitHandler, validateEmail } from '../Util/DOM';
import Config from '../Config/Config';
import Messages from '../Screens/Elements/Messages';
import Session from '../Session/Session';
import Sound from '../Sound';

let scheduled;

Screens.add('login', () => {
  // Build HTML
  Screens.update('login', `
    <div>
      <throbber style="margin-bottom: 1.5em">
        <h3 class="first">COREGROUNDS</h3>
      </throbber>
      <div class="menu text-center">
        <form id="login-form" autocomplete="off">
          <input type="text" id="login-mail" class="margin-bottom" placeholder="E-Mail-Address"></input>
          <input type="password" id="login-password" class="margin-bottom" placeholder="Password"></input>
          <button type="submit" class="box" id="login-submit">Login</button>
          <input type="submit" style="visibility: hidden; position: absolute;" />
        </form>
        <div style="margin: 2em 0;">
          <a href="${Config.ENDPOINT}/player/password" target="_blank">Forgot</a> your password?
        </div>
        <button class="box" onclick="Screens.show('create-account')">CREATE ACCOUNT</button>
        <button class="box grey" onclick="window.close();">QUIT</button>
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
    Session.login({
      username: $('#login-mail').value,
      password: $('#login-password').value,
    });
    return false;
  });
});

Screens.add('create-account', () => {
  // Build HTML
  Screens.update('create-account', `
    <div>
      <throbber style="margin-bottom: 1.5em">
        <h3 class="first">COREGROUNDS</h3>
      </throbber>
      <div class="menu">
      <form id="create-account-form" autocomplete="off">
        <div id="username-available">&nbsp;</div>
        <input type="text" id="create-name" class="margin-bottom" placeholder="Username" />
        <input type="text" id="create-mail" class="margin-bottom" placeholder="E-Mail-Address" />
        <input type="password" id="create-password" class="margin-bottom" placeholder="Password" />
        <input type="password" id="create-password-confirm" class="margin-bottom" placeholder="Confirm Password" />
        <input type="checkbox" id="tos" name="tos" value="0">
        <label class="option" for="tos">I agree to the <a target="_blank" href="${Config.ENDPOINT}/terms-of-use">Terms of Use</a> and the <a target="_blank" href="${Config.ENDPOINT}/privacy-policy">Privacy Policy</a>.</label>
        <input type="checkbox" id="news" name="news" value="0">
        <label class="option" for="news">Send me occasional emails about Coregrounds.</label>
        <button type="submit" class="box" id="create-submit" style="margin-top:.75em">Create account</button>
        <input type="submit" style="visibility: hidden; position: absolute;" />
        </form>
        <button class="box grey" onclick="Screens.show('login')">Back</button>
      </div>
    </div>
  `);

  // Add username check to the input field
  $('#create-name').on('keyup', checkUsername);

  // Attach submit handlers
  submitHandler('create-submit', () => {
    // Check if we have input in the mail field
    if (!$('#create-mail').value.length) {
      Messages.show('Please enter your email address.', 'error');
      Sound.play('error');
      return false;
    }
    // Check if we have input in the password field
    if (!$('#create-password').value.length) {
      Messages.show('Please enter your password.', 'error');
      Sound.play('error');
      return false;
    }
    // Check if we have a valid email address
    if (!validateEmail($('#create-mail').value)) {
      Messages.show('Please enter a valid email address.', 'error');
      Sound.play('error');
      return false;
    }
    // Check if the password fields are matching
    if ($('#create-password').value !==
        $('#create-password-confirm').value) {
      Messages.show('The passwords entered do not match.', 'error');
      Sound.play('error');
      return false;
    }
    if (!$('#tos').checked) {
      Messages.show('You have to accept the Terms of Use and the Privacy Policy.', 'error');
      Sound.play('error');
      return false;
    }
    return true;
  }, () => {
    // Execute login
    Session.create({
      name: $('#create-name').value,
      mail: $('#create-mail').value,
      pass: $('#create-password').value,
      news: $('#news').checked ? 1 : 0,
    });
    return false;
  });
});

/**
 * Helper function firing username check requests
 */
function checkUsername() {
  $('#username-available').innerHTML = 'CHECKING...';
  $('#username-available').className = '';
  // Clear any previously scheduled requests
  clearTimeout(scheduled);
  // Schedule a new request in 200ms
  scheduled = setTimeout(() => {
    request('app/check/name', {
      name: $('#create-name').value,
    }, (response) => {
      if (response.success) {
        $('#username-available').innerHTML = 'USERNAME AVAILABLE';
        $('#username-available').className = 'available';
      } else {
        $('#username-available').innerHTML = 'USERNAME TAKEN';
        $('#username-available').className = 'not-available';
      }
    });
  }, 200);
}
