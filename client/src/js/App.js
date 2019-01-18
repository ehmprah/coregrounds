/**
 * @file
 * The Mothership of an armada of beautiful ES6 modules.
 */
import Event from './Event';
import './Console/Console';
import './Game/Streaks';
import './Graphics/Graphics';
import './Music';
import './Screens/Screens';
import './Screens/Elements/Menu';

// Add all screens
// TODO add different solution, don't use those globals, otherwise we HAVE to include those after
// the general Screens.js
import './Screens/About';
import './Screens/Achievements';
import './Screens/Buy';
import './Screens/Changelog';
import './Screens/Development';
import './Screens/GameGuide';
import './Screens/Home';
import './Screens/Login';
import './Screens/MatchHistory';
import './Screens/Modifications';
import './Screens/News';
import './Screens/Onboarding';
import './Screens/Play';
import './Screens/Profile';
import './Screens/Redeem';
import './Screens/Quit';
import './Screens/Scoregrounds';
import './Screens/SelectOverlay';
import './Screens/Settings';
import './Screens/Stats';
import './Screens/Status';
import './Screens/Steam';
import './Screens/Store';
import './Screens/TosGate';
import './Screens/Tutorial';
import './Screens/TrainingGrounds';
import './Screens/Unlock';
import './Screens/Video';

window.addEventListener('load', () => {
  Event.fire('app loaded', 'after load', 'app ready');
});

// Add jQuery like selector and .on() method
window.$ = document.querySelector.bind(document);
window.$$ = document.querySelectorAll.bind(document);

Node.prototype.on = function on(name, fn) {
  this.addEventListener(name, fn);
};

window.on = function on(name, fn) {
  this.addEventListener(name, fn);
};

// eslint-disable-next-line
NodeList.prototype.__proto__ = Array.prototype;

NodeList.prototype.on = function on(name, fn) {
  this.forEach((elem) => {
    elem.on(name, fn);
  });
};

NodeList.prototype.addEventListener = function on(name, fn) {
  this.forEach((elem) => {
    elem.on(name, fn);
  });
};
