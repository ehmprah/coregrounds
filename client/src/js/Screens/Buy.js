/**
 * @file
 * The ACTUAL store where you can buy the proxy currency
 */
import Event from '../Event';
import Config from '../Config/Config';
import { request } from '../API';
import Analytics from '../Analytics';
import Messages from './Elements/Messages';
import Session from '../Session/Session';
import Sound from '../Sound';
import Icons from './Elements/Icons';
/* global greenworks */

let country;
let currency;
let steamid;
let language;
let overlayTimeout;

Event.on('session ready', () => {
  // Listen for authorization for microtransactions
  greenworks.on('micro-txn-authorization-response', (appid, orderid, authorized) => {
    clearTimeout(overlayTimeout);
    if (authorized) {
      Screens.update('buy', `
        <throbber>FINISHING TRANSACTION</throbber>
      `);
      request('app/steam/store/finalize', { orderid }, (response) => {
        Messages.show('Transaction completed!');
        Sound.play('win');
        // Update account
        const account = Session.getAccount();
        account.currency.current = response.currency;
        Session.setAccount(account);
        // Go to home screen
        Screens.show('store-colors');
      }, () => {
        Messages.show('Something went wrong.', 'error');
      });
      Analytics.event('Microtransactions', 'finalize');
    } else {
      Screens.show('buy');
      request('app/steam/store/cancel', { orderid });
      Analytics.event('Microtransactions', 'cancel');
    }
  });
});

Screens.add('buy', () => {
  // If we show the store page for the first time, we have to get additional info from the server
  if (!steamid) {
    Screens.update('buy', `
      <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <throbber>CONNECTING TO SERVER</throbber>
    `);
    // Get extended user info
    steamid = greenworks.getSteamId().steamId;
    language = greenworks.getCurrentGameLanguage();
    request('app/steam/store/userinfo', { steamid }, (response) => {
      country = response.params.country;
      currency = response.params.currency;
      Screens.show('buy');
    });
    return;
  }

  let formatted = '';
  let specials = '';

  // Get available bundles
  const bundles = Config.query('purchases', { available: true, special: false }, true);
  bundles.forEach((bundle) => {
    // Get price from config
    const price = bundle.prices[currency] !== undefined
      ? `<amount>${(bundle.prices[currency] / 100).toFixed(2)}</amount> ${currency.toUpperCase()}`
      : `<amount>${(bundle.prices.USD / 100).toFixed(2)}</amount> USD`;
    // Build HTML accordingly
    formatted += `
      <li>
        <unlock class="buy blackbox bundle-${bundle.id}" data-amount="${bundle.currency}">
          ${bundle.popular ? '<label class="info black-alpha">most popular</label>' : ''}
          ${bundle.bestValue ? '<label class="info black-alpha">best value</label>' : ''}
          <sprite>
            <div class="coin">
              <div class="front-inner"></div>
              <div class="back-inner"></div>
              <div class="front"></div>
              <div class="back"></div>
            </div>
          </sprite>
          <label class="amount">${bundle.currency}</label>
          <button class="box unlock-btn vertical" onclick="initMicrotransaction(${bundle.id})">
            <currency class="money">
              ${price}
            </currency>
          </button>
        </unlock>
      </li>
    `;
  });

  // Build HTML accordingly
  const special = Config.query('purchases', { id: 6 });
  // Get price from config
  const price = special.prices[currency] !== undefined
    ? `<amount>${(special.prices[currency] / 100).toFixed(2)}</amount> ${currency.toUpperCase()}`
    : `<amount>${(special.prices.USD / 100).toFixed(2)}</amount> USD`;
  specials += `
    <div class="blackbox clearfix special-package">
      <div class="info">
        <img src="player-icons/2004.png">
        <h4>INDIE SUPPORTER PACKAGE</h4>
        <ul>
          <li>+ Exclusive player icon ("Heart of Gold")</li>
          <li>+ Exclusive player color ("Gold")</li>
          <li>+ Exclusive role on Discord & flair on Subreddit ("Supporter")</li>
        </ul>
      </div>
      <currency class="currency">
        ${Icons.get('currency')}
        <amount>${special.currency}</amount>
      </currency>
      <button class="box unlock-btn vertical" onclick="initMicrotransaction(${special.id})">
        <currency class="money">
          ${price}
        </currency>
      </button>
    </div>
  `;

  Screens.update('buy', `
    <div class="container">
      <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content">
        <h2 class="text-center">Buy Cordium</h2>
        <ul class="unlocks content-row-${bundles.length}">
          ${formatted}
        </ul>
        ${specials}
      </div>
    </div>
  `);
});

window.initMicrotransaction = function initMicrotransaction(bundle) {
  Screens.update('buy', `
    <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
    <throbber>PROCESSING TRANSACTION</throbber>
  `);

  Analytics.event('Microtransactions', 'init', 'bundle', bundle);
  request('app/steam/store/init', {
    bundle,
    country,
    currency,
    language,
    steamid,
  });

  overlayTimeout = setTimeout(() => {
    Screens.update('buy', `
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <throbber>
        <p>PROCESSING TRANSACTION</p>
        <p>Steam overlay not opening?</p>
        <div class="menu" style="min-width: 280px;">
          <button class="box" id="web-based">CONTINUE IN BROWSER</button>
        </div>
      </throbber>
    `);
    document.getElementById('web-based').addEventListener('click', () => {
      Screens.update('buy', `<throbber>CREATING WEB-BASED TRANSACTION</throbber>`);
      Analytics.event('Microtransactions', 'web', 'bundle', bundle);
      request('app/steam/store/init', {
        bundle,
        country,
        currency,
        language,
        steamid,
        web: true,
      }, (data) => {
        Screens.update('buy', `
          <throbber>
            <p>TRANSACTION READY</p>
            <div class="menu" style="min-width: 280px;">
              <button class="box" onclick="window.open('${data.steamurl}?returnurl=${Config.ENDPOINT}app/steam/store/finalize/${data.orderid}'); Screens.show('buy-refresh')">
                OPEN BROWSER
              </button>
            </div>
          </throbber>
        `);
      });
    });
  }, 2000);
};

Screens.add('buy-refresh', () => {
  Screens.update('buy-refresh', `
    <throbber>
      <p>TRANSACTION FINISHED?</p>
      <div class="menu" style="min-width: 280px;">
        <button class="box" id="transaction-refresh">DONE</button>
      </div>
    </throbber>
  `);
  document.getElementById('transaction-refresh').addEventListener('click', () => {
    Session.reload();
    Screens.show('home');
  });
});
