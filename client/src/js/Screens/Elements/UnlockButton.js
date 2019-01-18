/**
 * @file UnlockButton.js
 * Builds buttons with prices on them
 */
import icon from '../../../img/icons/icon-credits.svg';

export default {
  get: function(id, label, price) {
    return `
      <button class="box unlock-btn" id="${id}">
        <span>${label}</span>
        <currency class="credits">
          ${icon}
          <amount>${price}</amount>
        </currency>
      </button>
    `;
  }
}
