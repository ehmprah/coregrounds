/**
 * @file
 * The quit submenu
 *
 * TODO Add messages regarding their recent results, reinforcing good sessions.
 */
import { arrayRandom } from '../Util/Mixed';

const msgs = [
  'We will miss you!',
  'Wait, what?',
  'Hold it right there!',
  'But you are coming back, right?',
  'Already?!',
  'RLY?',
  'Nooooo, come back!',
  'Please don\'t go!',
  'Goodbye, for now!',
  'Just. One. More. Game.',
  'Until the next time on the Coregounds!',
];

Screens.add('quit', () => {
  Screens.update('quit', `
    <div class="menu">
      <throbber>${arrayRandom(msgs)}</throbber>
      <button class="box red" id="btn-quit">QUIT</button>
      <button class="box grey" onclick="Screens.show('home');">CANCEL</a>
    </div>
  `);

  $('#btn-quit').on('click', () => {
    window.close();
  });
});
