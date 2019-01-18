/**
 * @file Store.js
 * Store screens
 *
 * TODO ADD STARTER PACK WITH A BIT OF EVERYTHING!
 */
import { addThousandSeparator } from '../Util';
import { buildTabs } from '../Util/DOM';
import { request } from '../API';
import Analytics from '../Analytics';
import Config from '../Config/Config';
import Icons from '../Screens/Elements/Icons';
import Messages from '../Screens/Elements/Messages';
import Session from '../Session/Session';
import Sprites from '../Graphics/Sprites';
import Sound from '../Sound';
import Wallet from './Elements/Wallet';

const stores = {
  'unlock-towers': 'Credits',
  'store-colors': 'Cordium',
};

const screens = {
  'store-colors': 'Colors',
  'store-skins': 'Skins',
  'store-icons': 'Icons',
  'store-backgrounds': 'Backgrounds',
  'store-emotes': 'Emotes',
  'store-credits': 'Credits',
};

/**
 * Add store screen for credits and boosts
 */
Screens.add('store-credits', () => {
  const credits = Config.get('credits');

  Screens.update('store-credits', `
    <div class="container-full">
      <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content">
        ${buildTabs(stores, 'store-colors')}
        ${buildTabs(screens, 'store-credits')}
        <ul class="content-row-3">
          <li>
            <unlock class="bonus blackbox">
              <label>BONUS</label>
              <sprite>${Icons.get('bonus')}</sprite>
              <label class="text-silver">+100% Credits</label>
              <label class="amount">for ${credits[0].bonusWins} wins</label>
              ${getBtnPurchase('credits', credits[0].price, credits[0].id)}
            </unlock>
          </li>
          <li>
            <unlock class="bonus blackbox">
              <label>BONUS</label>
              <sprite>${Icons.get('bonus')}</sprite>
              <label class="text-silver">+100% Credits</label>
              <label class="amount">for ${credits[1].bonusHours} hours</label>
              ${getBtnPurchase('credits', credits[1].price, credits[1].id)}
            </unlock>
          </li>
          <li>
            <unlock class="bonus blackbox">
              <label>BONUS</label>
              <sprite>${Icons.get('bonus')}</sprite>
              <label class="text-silver">+${credits[2].bonusPermanent}% Credits</label>
              <label class="amount">forever</label>
              ${getBtnPurchase('credits', credits[2].price, credits[2].id)}
            </unlock>
          </li>
        </ul>
        <ul class="content-row-4">
          <li>
            <unlock class="credits blackbox">
              <label>CREDITS</label>
              <sprite>${Icons.get('credits')}</sprite>
              <label class="amount">
                ${addThousandSeparator(credits[3].flatCredits)}
              </label>
              ${getBtnPurchase('credits', credits[3].price, credits[3].id)}
            </unlock>
          </li>
          <li>
            <unlock class="credits blackbox">
              <label>CREDITS</label>
              <sprite>${Icons.get('credits')}</sprite>
              <label class="amount">
                ${addThousandSeparator(credits[4].flatCredits)}
              </label>
              ${getBtnPurchase('credits', credits[4].price, credits[4].id)}
            </unlock>
          </li>
          <li>
            <unlock class="credits blackbox">
              <label>CREDITS</label>
              <sprite>${Icons.get('credits')}</sprite>
              <label class="amount">
                ${addThousandSeparator(credits[5].flatCredits)}
              </label>
              ${getBtnPurchase('credits', credits[5].price, credits[5].id)}
            </unlock>
          </li>
          <li>
            <unlock class="credits blackbox">
              <label>CREDITS</label>
              <sprite>${Icons.get('credits')}</sprite>
              <label class="amount">
                ${addThousandSeparator(credits[6].flatCredits)}
              </label>
              ${getBtnPurchase('credits', credits[6].price, credits[6].id)}
            </unlock>
          </li>
        </ul>
        <div style="text-align: center; margin: 2em 0 0;">
          Did you know? You get a 50% credit bonus when you're
          <a href="#" onclick="Screens.show('settings-account')">subscribed to the newsletter</a>!
        </div>
      </div>
    </div>
    ${Wallet.get()}
  `);
});

/**
 * Add store screen for colors
 */
Screens.add('store-colors', () => {
  let items = '';
  Config.get('colors').forEach((item) => {
    if (item.unlockable) {
      items += `
        <li>
          <unlock class="blackbox color-${item.id}${unlockClasses(item)}" id="unlock-${item.id}">
            <label class="text-${item.id}">${item.name}</label>
            ${Sprites.withBase(1103, item.main)}
            ${getBtnPurchase('colors', item.price, item.id)}
          </unlock>
        </li>
      `;
    }
  });
  // Build screen
  Screens.update('store-colors', `
    <div class="container-full">
      <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content">
        ${buildTabs(stores, 'store-colors')}
        ${buildTabs(screens, 'store-colors')}
        <ul class="unlocks content-row-4">${items}</ul>
      </div>
    </div>
    ${Wallet.get()}
  `);
});

/**
 * Add store screen for colors
 */
Screens.add('store-skins', () => {
  let items = '';
  // Prepare items
  Config.get('skins').forEach((item) => {
    if (item.unlockable) {
      items += `
        <li>
          <unlock class="skin blackbox${unlockClasses(item)}" id="unlock-${item.id}">
            <label>${item.name}</label>
            ${Sprites.withBase(item.id)}
            ${getBtnPurchase('skins', item.price, item.id)}
          </unlock>
        </li>
      `;
    }
  });
  // Build screen
  Screens.update('store-skins', `
    <div class="container-full">
      <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content">
        ${buildTabs(stores, 'store-colors')}
        ${buildTabs(screens, 'store-skins')}
        <ul class="unlocks content-row-4">${items}</ul>
      </div>
    </div>
    ${Wallet.get()}
  `);
});

/**
 * Add store screen for colors
 */
Screens.add('store-emotes', () => {
  let items = '';
  // Prepare items
  Config.get('emotes').forEach((item) => {
    if (item.unlockable) {
      items += `
        <li>
          <unlock class="emote blackbox${unlockClasses(item)}" id="unlock-${item.id}">
            <div class="flex"><span>"${item.emote}"</span></div>
            ${getBtnPurchase('emotes', item.price, item.id)}
          </unlock>
        </li>
      `;
    }
  });
  // Build screen
  Screens.update('store-emotes', `
    <div class="container-full">
      <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content">
        ${buildTabs(stores, 'store-colors')}
        ${buildTabs(screens, 'store-emotes')}
        <ul class="unlocks content-row-4">${items}</ul>
      </div>
    </div>
    ${Wallet.get()}
  `);
});

/**
 * Add store screen for colors
 */
Screens.add('store-icons', () => {
  let items = '';
  Config.get('playerIcons').forEach((item) => {
    if (item.unlockable) {
      items += `
        <li>
          <unlock class="icon blackbox${unlockClasses(item)}" id="unlock-${item.id}">
            <label>${item.name}</label>
            <div class="player-icon">
              <img src="player-icons/${item.file}">
            </div>
            ${getBtnPurchase('playerIcons', item.price, item.id)}
          </unlock>
        </li>
      `;
    }
  });
  // Build screen
  Screens.update('store-icons', `
    <div class="container-full">
      <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content">
        ${buildTabs(stores, 'store-colors')}
        ${buildTabs(screens, 'store-icons')}
        <ul class="unlocks content-row-4">${items}</ul>
      </div>
    </div>
    ${Wallet.get()}
  `);
});

/**
 * Add store screen for colors
 */
Screens.add('store-backgrounds', () => {
  let items = '';
  // Prepare items
  Config.get('backgrounds').forEach((item) => {
    if (item.unlockable) {
      // And build the HTML accordingly
      items += `
        <li>
          <unlock class="bg blackbox background-${item.id}${unlockClasses(item)}" id="unlock-${item.id}">
            <label>${item.name}</label>
            ${getBtnPurchase('backgrounds', item.price, item.id)}
          </unlock>
        </li>
      `;
    }
  });
  // Build screen
  Screens.update('store-backgrounds', `
    <div class="container-full">
      <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content">
        ${buildTabs(stores, 'store-colors')}
        ${buildTabs(screens, 'store-backgrounds')}
        <ul class="unlocks content-row-3">${items}</ul>
      </div>
    </div>
    ${Wallet.get()}
  `);
});

function getBtnPurchase(type, price, id) {
  return `
    <button class="box unlock-btn vertical" data-price="${price}" onclick="btnPurchase(this, '${type}', ${id})">
      <currency class="currency">
        ${Icons.get('currency')}
        <amount>${price}</amount>
      </currency>
    </button>
  `;
}

window.btnPurchase = function btnPurchase(btn, type, id) {
  // Show error and stop before the request if we have insufficient credits
  if (Session.currency() < btn.dataset.price) {
    Messages.show('Insufficient funds.', 'warning');
    Sound.play('error');
    return;
  }

  // Add throbber class to button
  btn.classList.add('waiting');
  // Send request to webserver
  request('app/buy', {
    type,
    id,
  }, (response) => {
    Analytics.event('Store', 'buy', type, id);
    Messages.show(response.message);
    Sound.play('win');
    Screens.refresh();
    btn.classList.remove('waiting');
  }, () => {
    btn.classList.remove('waiting');
  });
};

function unlockClasses(item) {
  const account = Session.getAccount();
  let classes = '';
  if (account.unlocks.indexOf(item.id) > -1) {
    classes += ' unlocked';
  } else if (item.price > account.currency.current) {
    classes += ' grey';
  }
  return classes;
}
