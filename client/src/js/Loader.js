/**
 * @file Loader.js
 * Handles loading tasks and displays loading progress
 *
 * @see http://www.pindi.us/blog/preventing-css-transition-blocking-js-code
 * @todo add progress by, dividing current by all tasks
 */
import Event from './Event';

const tasks = {}, total = 0, current = 0;

export default {
  task: function(label, callback) {
    // Group tasks by label
    if (tasks[label] == undefined) {
      tasks[label] = [];
    }

    tasks[label].push(callback);
  }
}

window.addEventListener('load', function(event) {

  // Process all tasks
  deferLoop();


  Event.fire('app loaded', 'after load', 'app ready');
});

function deferLoop(callbacks) {
  let d = new Date();
  while ((new Date()) - d < 15) {
    let f = callbacks.shift();
    if (!f) return;
    f();
  }
  setTimeout(function(){
    deferLoop(callbacks)
  }, 0);
}
