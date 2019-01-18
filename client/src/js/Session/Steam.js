/**
 * @file
 * Steam session handling
 */
import { request } from '../API';
import Session from './Session';
/* global greenworks */

export default {
  auth() {
    // Show new status page
    Screens.show('status', 0, 'AUTHENTICATING');
    // Make sure we have an initialized steam api
    if (!greenworks.initAPI()) {
      return Screens.show('steam-error');
    }
    // Send session ticket based request
    steamrequest('app/steam/auth', {}, (response) => {
      if (response.player) {
        // If the response contains a player object, we can wrap up the authentication
        Session.create(response);
      } else {
        // Otherwise we have to create a new one or link an account to the steamid
        Screens.show('steam-init');
      }
    }, (response) => {
      if (response.maintenance) {
        Screens.show('maintenance');
      } else {
        Screens.show('auth-error');
      }
    });
  },

  init(username, onFailure) {
    steamrequest('app/steam/init', { username }, (response) => {
      // We can safely assume a session here
      Session.create(response, true);
    }, onFailure);
  },

  link(username, password, onFailure) {
    steamrequest('app/steam/link', { username, password }, (response) => {
      // We can safely assume a session here
      Session.create(response);
    }, onFailure);
  },
};

function steamrequest(url, data, onSuccess, onFailure = false) {
  // Authenticate against the steam servers
  greenworks.getAuthSessionTicket((session) => {
    // Add session ticket
    data.ticket = session.ticket.toString('hex');
    // Send request to webserver
    request(url, data, (response) => {
      onSuccess(response);
      greenworks.cancelAuthTicket(session.handle);
    }, (response) => {
      if (onFailure) onFailure(response);
      greenworks.cancelAuthTicket(session.handle);
    });
  }, (error) => {
    Screens.show('error', 0, error);
  });
}
