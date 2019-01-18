/**
 * @file ConsoleHistory.js
 * Helper providing a history to cycle through in the console
 */
import Event from '../Event';
import Config from '../Config/Config';
import Game from '../Game/Game';
import Keyboard from '../Keyboard';
import { adoptHTML } from '../Util';

let history = [];
let index = 0;

// Try to load the history from the local storage
let stored = window.localStorage.getItem('console-history');
if (stored) {
  history = JSON.parse(stored);
}

export default {

  next: function() {
    if (!history.length) return '';
    // Adjust index, cycling through to the beginning at the end
    index++;
    if (index >= history.length) {
      index = 0;
    }
    return history[index];
  },

  last: function() {
    if (!history.length) return '';
    index--;
    if (index < 0) {
      index += history.length;
    }console.log(history);
    return history[index];
  },

  add: function(item) {
    // We also reset the index to avoid confusion
    index = 0;
    // Add current item to the history
    history.push(item);
    // Limit the history to ten items
    if (history.length > 10) {
      history.splice(0, 1);
    }
    // Save the new history
    window.localStorage.setItem('console-history', JSON.stringify(history));
  }
}
