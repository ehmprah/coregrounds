/**
 * @file
 * Handles API requests against the web server
 */
import Config from './Config/Config';
import Event from './Event';
import Messages from './Screens/Elements/Messages';
import Session from './Session/Session';
import Sound from './Sound';
import { logToConsole } from './Console/Console';

let token;

Event.on('auth ready', () => {
  token = Session.getToken();
});

export function request(url, post, onSuccess = false, onFailure = false, returnAsap = false) {
  const ajax = new XMLHttpRequest();
  const params = [];

  ajax.addEventListener('error', () => {
    logToConsole(`Error sending request: ${JSON.stringify(ajax)}`, 'error');
    Screens.show('error', 0, `Error sending request`);
  });

  ajax.onreadystatechange = () => {
    if (ajax.readyState === XMLHttpRequest.DONE) {
      if (ajax.status === 200) {
        let data = {};
        try {
          data = JSON.parse(ajax.response);
        } catch (e) {
          logToConsole(`Error sending request: ${JSON.stringify(ajax)}`, 'error');
        }

        if (returnAsap) return onSuccess(data);

        // Automatically show error messages
        // (and do NOT execute the successCallback)
        if (!data.success) {
          if (data.message) {
            Messages.show(data.message, 'error');
            Sound.play('error');
          }
          if (onFailure) {
            onFailure(data);
          }
          return;
        }
        // Handle the success callback if we have one
        if (onSuccess) {
          // We only handle player and achievement updates if we have a valid session
          if (Session.ready()) {
            if (data.player) {
              if (Session.getAccount().level.current < data.player.level.current) {
                Messages.show('Level up!');
                Sound.play('win');
              }
              Session.setAccount(data.player);
            }
            if (data.achievements && data.achievements.length) {
              Screens.show('achievements-unlocked', 1, data.achievements);
            }
          }
          // Then handle the callback
          onSuccess(data);
        }
      }
    }
  };

  const customApiServer = Config.User.get('api-server');
  if (customApiServer) {
    ajax.open('POST', customApiServer + url, true);
  } else {
    ajax.open('POST', Config.ENDPOINT + url, true);
  }

  ajax.setRequestHeader(
    'Content-type',
    'application/x-www-form-urlencoded'
  );

  if (token) {
    params.push(`token=${token}`);
  }

  const keys = Object.keys(post);
  if (keys.length) {
    keys.forEach((key) => {
      params.push(`${key}=${post[key]}`);
    });
  }

  ajax.send(params.join('&'));
}
