/**
 * @file
 * Builds buttons for ecommerce purchases and unlocks
 */
import { request } from '../../API';
import Session from '../../Session/Session';
import Sound from '../../Sound';
import Messages from './Messages';
import Analytics from '../../Analytics';
import Icons from '../Elements/Icons';

export function getBtnUnlock(id, price) {
  return `
    <button class="box unlock-btn vertical" data-price="${price}" onclick="btnUnlock(${id})">
      <span>Unlock</span>
      <currency class="credits">
        ${Icons.get('credits')}
        <amount>${price}</amount>
      </currency>
    </button>
  `;
}

window.btnUnlock = (id) => {
  const btn = $(`#unlock-${id} .unlock-btn`);

  // Show error and stop before the request if we have insufficient credits
  if (Session.credits() < btn.dataset.price) {
    Messages.show('Insufficient credits.', 'warning');
    Sound.play('error');
    return;
  }

  // Add throbber class to button
  btn.classList.add('waiting');
  // Send request to webserver
  request('app/unlock', { id }, (response) => {
    Analytics.event('Unlock', 'unlock', id, id);
    Messages.show(response.message);
    Sound.play('win');
    // Stop the throbbing
    btn.classList.remove('waiting');
  });
};
