/**
 * @file Wallet.js
 * Shows the user's current currency balances.
 */
import Session from '../../Session/Session';
import Icons from './Icons';
// import Event from './Icons';

export default {
  get() {
    // Load account
    const account = Session.getAccount();
    return `
      <div class="wallet">
        <currency class="player credits" onclick="Screens.show('store-credits')">
          ${Icons.get('credits')}
          <amount>${account.credits.current}</amount>
        </currency>
        <currency class="player currency" onclick="Screens.show('buy')">
          ${Icons.get('currency')}
          <amount>${account.currency.current}</amount>
        </currency>
      </div>
    `;
  },
};

// Event.fire('screen rendered');
