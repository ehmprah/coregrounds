/**
 * @file
 * Authentication and session handling
 *
 * TODO can we safely remove the local storage now?
 */
import Event from '../Event';
import Messages from '../Screens/Elements/Messages';
import Config from '../Config/Config';
import Sound from '../Sound';
import { request } from '../API';
// Store implementations
import Steam from './Steam';

let token;
let account;
let ready = false;
let unlockedAchievements;

let isOnboarding = false;

export default {

  ready() {
    return ready;
  },

  getToken() {
    return token;
  },

  getAccount() {
    return account;
  },

  setAccount(data) {
    account = JSON.parse(JSON.stringify(data));
  },

  reload() {
    request('app/reload', false, () => {});
  },

  credits() {
    return account.credits.current;
  },

  currency() {
    return account.currency.current;
  },

  finishOnboarding() {
    isOnboarding = false;
  },

  isOnboarding() {
    return isOnboarding;
  },

  create(response, onboarding = false) {
    // Save session token in variable
    ({ token } = response);
    // Save account data in variable and local storage
    this.setAccount(response.player);
    // Pass on config and save it in local storage
    Config.update(JSON.parse(response.config));
    window.localStorage.setItem('config', response.config);
    if (response.achievements && response.achievements.length) {
      unlockedAchievements = response.achievements;
    }
    // Allow for api requests before finalizing the session
    Event.fire('auth ready');
    if (account.tos_updated) {
      Screens.show('tosgate', 1, account.tos_updated);
    } else if (onboarding) {
      isOnboarding = true;
      ready = true;
      Event.fire('session ready');
      Screens.show('choose-region');
    } else {
      finalize();
    }
  },
};

export function finalize() {
  // Show welcome message
  Messages.show(`Welcome, ${account.name}!`);
  Sound.play('toggle_interface');
  // Set ready status
  ready = true;
  // Tell other modules we're ready here
  Event.fire('session ready');
  // Show home screen after
  Screens.show('home');
  // And check for achievemnts we might have unlocked
  if (unlockedAchievements) {
    Screens.show('achievements-unlocked', 1, unlockedAchievements);
    unlockedAchievements = false;
  }
  // eslint-disable-next-line
  console.log(account);
}

Event.on('after load', () => {
  // TODO add google play implementation
  // TODO add app store implementation
  Steam.auth();
});

Event.on('session ready', () => {
  // Reset the body classes
  $('body').className = '';
  // Set body color on the body
  setBodyClass('color', account.color);
  // Set background on the body
  if (account.background) {
    setBodyClass('background', account.background);
  }
});
