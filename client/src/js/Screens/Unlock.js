/**
 * @file
 * Screen with unlocks for towers, minions, abilities and modifications.
 */
import { buildTabs } from '../Util/DOM';
import { request } from '../API';
import Analytics from '../Analytics';
import Config from '../Config/Config';
import Icons from './Elements/Icons';
import Messages from './Elements/Messages';
import Session from '../Session/Session';
import Sound from '../Sound';
import Sprites from '../Graphics/Sprites';
import Wallet from './Elements/Wallet';

const stores = {
  'unlock-towers': 'Credits',
  'store-colors': 'Cordium',
};

const screens = {
  'unlock-towers': 'Towers',
  'unlock-minions': 'Minions',
  'unlock-abilities': 'Abilities',
  'unlock-modifications-offensive': 'Modifications',
};

const modTabs = {
  'unlock-modifications-offensive': 'Offensive',
  'unlock-modifications-defensive': 'Defensive',
  'unlock-modifications-utility': 'Utility',
};

Screens.add('unlock-towers', () => {
  let items = '';
  Config.get('factories').forEach((item) => {
    if (item.tower && item.unlockable) {
      // And build HTML accordingly
      items += `
        <li>
          <unlock class="blackbox${unlockClasses(item)}" id="unlock-${item.id}">
            <label>${item.name}</label>
            ${Sprites.withBase(item.id)}
            ${getBtnUnlock(item.id, item.price)}
            ${gameGuideBtn(item.id)}
          </unlock>
        </li>
      `;
    }
  });

  // Build screen
  Screens.update('unlock-towers', `
    <div class="container-full">
      <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content">
        ${buildTabs(stores, 'unlock-towers')}
        ${buildTabs(screens, 'unlock-towers')}
        <ul class="unlocks content-row-4">${items}</ul>
      </div>
    </div>
    ${Wallet.get()}
  `);
});

Screens.add('unlock-minions', () => {
  let items = '';
  Config.get('factories').forEach((item) => {
    if (item.minion && item.unlockable) {
      items += `
        <li>
          <unlock class="blackbox${unlockClasses(item)}" id="unlock-${item.id}">
            <label>${item.name}</label>
            ${Sprites.withBase(item.id)}
            ${getBtnUnlock(item.id, item.price)}
            ${gameGuideBtn(item.id)}
          </unlock>
        </li>
      `;
    }
  });

  // Build screen
  Screens.update('unlock-minions', `
    <div class="container-full">
      <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content">
        ${buildTabs(stores, 'unlock-towers')}
        ${buildTabs(screens, 'unlock-minions')}
        <ul class="unlocks content-row-4">${items}</ul>
      </div>
    </div>
    ${Wallet.get()}
  `);
});

Screens.add('unlock-abilities', () => {
  let items = '';
  Config.get('abilities').forEach((item) => {
    if (item.unlockable) {
      // And build HTML accordingly
      items += `
        <li>
          <unlock class="blackbox${unlockClasses(item)}" id="unlock-${item.id}">
            <label>${item.name}</label>
            ${Sprites.withBase(item.id)}
            ${getBtnUnlock(item.id, item.price)}
            ${gameGuideBtn(item.id)}
          </unlock>
        </li>
      `;
    }
  });

  // Build screen
  Screens.update('unlock-abilities', `
    <div class="container-full">
      <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content">
        ${buildTabs(stores, 'unlock-towers')}
        ${buildTabs(screens, 'unlock-abilities')}
        <ul class="unlocks content-row-4">${items}</ul>
      </div>
    </div>
    ${Wallet.get()}
  `);
});

Screens.add('unlock-modifications-offensive', () => {
  let items = '';
  Config.get('modifications').forEach((item) => {
    if (item.unlockable && item.group === 0) {
      // And build HTML accordingly
      items += `
        <li>
          <unlock class="blackbox${unlockClasses(item)}" id="unlock-${item.id}">
            <div class="flex" style="min-height: 180px;">
              <div class="text">${item.name}</div>
            </div>
            <label class="clr-text">${item.target}</label>
            <label style="color: #888; margin-bottom: 1em;">OFFENSIVE</label>
            ${getBtnUnlock(item.id, item.price)}
          </unlock>
        </li>
      `;
    }
  });
  // Build screen
  Screens.update('unlock-modifications-offensive', `
    <div class="container-full">
      <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content">
        ${buildTabs(stores, 'unlock-towers')}
        ${buildTabs(screens, 'unlock-modifications-offensive')}
        ${buildTabs(modTabs, 'unlock-modifications-offensive')}
        <ul class="unlocks content-row-4">${items}</ul>
      </div>
    </div>
    ${Wallet.get()}
  `);
});

Screens.add('unlock-modifications-defensive', () => {
  let items = '';
  Config.get('modifications').forEach((item) => {
    if (item.unlockable && item.group === 1) {
      // And build HTML accordingly
      items += `
        <li>
          <unlock class="blackbox${unlockClasses(item)}" id="unlock-${item.id}">
            <div class="flex" style="min-height: 180px;">
              <div class="text">${item.name}</div>
            </div>
            <label class="clr-text">${item.target}</label>
            <label style="color: #888; margin-bottom: 1em;">DEFENSIVE</label>
            ${getBtnUnlock(item.id, item.price)}
          </unlock>
        </li>
      `;
    }
  });
  // Build screen
  Screens.update('unlock-modifications-defensive', `
    <div class="container-full">
      <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content">
        ${buildTabs(stores, 'unlock-towers')}
        ${buildTabs(screens, 'unlock-modifications-offensive')}
        ${buildTabs(modTabs, 'unlock-modifications-defensive')}
        <ul class="unlocks content-row-4">${items}</ul>
      </div>
    </div>
    ${Wallet.get()}
  `);
});

Screens.add('unlock-modifications-utility', () => {
  let items = '';
  Config.get('modifications').forEach((item) => {
    if (item.unlockable && item.group === 2) {
      // And build HTML accordingly
      items += `
        <li>
          <unlock class="blackbox${unlockClasses(item)}" id="unlock-${item.id}">
            <div class="flex" style="min-height: 180px;">
              <div class="text">${item.name}</div>
            </div>
            <label class="clr-text">${item.target}</label>
            <label style="color: #888; margin-bottom: 1em;">UTILITY</label>
            ${getBtnUnlock(item.id, item.price)}
          </unlock>
        </li>
      `;
    }
  });
  // Build screen
  Screens.update('unlock-modifications-utility', `
    <div class="container-full">
      <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content">
        ${buildTabs(stores, 'unlock-towers')}
        ${buildTabs(screens, 'unlock-modifications-offensive')}
        ${buildTabs(modTabs, 'unlock-modifications-utility')}
        <ul class="unlocks content-row-4">${items}</ul>
      </div>
    </div>
    ${Wallet.get()}
  `);
});

export function getBtnUnlock(id, price) {
  return `
    <button class="box unlock-btn vertical" data-price="${price}" onclick="btnUnlock(${id})">
      <currency class="credits">
        ${Icons.get('credits')}
        <amount>${price}</amount>
      </currency>
    </button>
  `;
}

export function gameGuideBtn(id) {
  return `
    <button
      class="box grey"
      style="margin-top: .25em; justify-content: center"
      onclick="Screens.show('game-guide-detail', 1, ${id})"
    >
      GAME GUIDE
    </button>
  `;
}

window.btnUnlock = function btnUnlock(id, cb) {
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
    // TODO add item category here instead of the id twice!
    Analytics.event('Unlocks', 'unlock', id, id);
    Messages.show(response.message);
    Sound.play('win');
    if (cb) {
      cb();
    } else {
      Screens.refresh();
    }
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
  } else if (item.price > account.credits.current) {
    classes += ' grey';
  }
  return classes;
}
