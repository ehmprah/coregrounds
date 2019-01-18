/**
 * @file
 * Adds screens to the app
 */
import Analytics from '../Analytics';
import Event from '../Event';
import Keyboard from '../Keyboard';
import Game from '../Game/Game';
import Session from '../Session/Session';
import UserInterface from '../Interface/UserInterface';

// The screen repository holding ids and callbacks
const screens = {};

// We keep track of the active screen per layer
const current = ['', ''];

const Screens = {

  current(layer = 0) {
    return current[layer];
  },

  add(id, callback) {
    // Add params to the screen repository
    screens[id] = callback;
  },

  show(id, layer = 0, ...args) {
    // Tell the event system about the screen change
    if (current[layer]) {
      Event.fire(`leaving ${current[layer]}`);
      Event.fire(`from ${current[layer]} to ${id}`);
    }
    Event.fire(`showing ${id}`);

    // Show the screen element (we do this via className as opposed to classList
    // here in order to reset other classes that have been set previously)
    $(`screen#layer-${layer}`).className = 'active';

    // Keep track of which screen we currently show
    current[layer] = id;

    // Run the screen builder with rest params handed to this function
    screens[id](...args.concat([layer]));

    // Send render finish event
    Event.fire('screen rendered');

    // Send screenview to analytics
    Analytics.screenView(id);
  },

  update(id, html, failureCallback, ...args) {
    let failed = true;
    for (let layer = 0; layer < current.length; layer++) {
      // We only update the screen if we're still on the one triggering the
      // update, so we don't do that after the user left a page which was still
      // waiting for a response from the server.
      if (current[layer] === id) {
        $(`screen#layer-${layer}`).innerHTML = html;
        // Send render finish event
        Event.fire('screen rendered');
        failed = false;
      }
    }
    // Run the failureCallback with extra params handed to this function
    if (failed && failureCallback) {
      failureCallback(...args);
    }

    return this;
  },

  addClass(cls, layer = 0) {
    $(`screen#layer-${layer}`).classList.add(cls);
  },

  refresh(layer = 0, ...args) {
    // Just refresh the page without any extra stuff
    screens[current[layer]](...args);
  },

  hide(layer = 0) {
    Event.fire(`leaving ${current[layer]}`);
    current[layer] = '';
    $(`screen#layer-${layer}`).classList.remove('active');
  },

  hideAll() {
    for (let layer = 0; layer < current.length; layer++) {
      this.hide(layer);
    }
  },

  close() {
    if (Session.isOnboarding()) return;
    if (Screens.current(0).indexOf('tutorial') > -1) return;

    if (Game.status) {
      if (Screens.current(1)) {
        return Screens.hide(1);
      }
      return UserInterface.reset();
    }

    // TODO this should also happen on the back btn for mobiles!
    // We try to hide the first layer
    if (Screens.current(1)) {
      return Screens.hide(1);
    }
    // Otherwise we hide the bottom layer
    Screens.hide();
    // If there's nothing left, show home!
    if (!Screens.current() && !Game.running()) {
      Screens.show('home');
    }
  },
};

// On game start, we close all overlays and pages
Event.on('game ready', Screens.hide);

Keyboard.on(27, Screens.close);

window.Screens = Screens;

export default Screens;
