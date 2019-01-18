/**
 * @file
 * Shows messages in the DOM
 *
 * TODO add "link" option to replace the onclick handler and send to pages?
 */
import { adoptHTML } from '../../Util';
import Event from '../../Event';

const stack = [];

Event.on('app loaded', () => {
  adoptHTML(`<div id="messages"></div>`);
});

const Messages = {
  show(msg, type = 'status') {
    // Create message element
    const message = document.createElement('div');

    // Add classes
    message.classList.add('message');
    message.classList.add(type);

    // Add message body
    message.innerHTML = msg;

    // Add it to the stack
    stack.push(message);

    // And get the next one from the stack if we currently don't have one
    if (!$('#messages').hasChildNodes()) next();
  },
};

function hide(message) {
  // Start CSS animation hiding the thing
  message.classList.add('remove');
  // And remove it after a while
  setTimeout(() => {
    if (message) {
      message.parentNode.removeChild(message);
    }
    next();
  }, 1000);
}

function next() {
  if (stack.length) {
    const message = stack.shift();
    $('#messages').appendChild(message);
    setTimeout(() => {
      hide(message);
    }, 3000);
  }
}

export default Messages;
