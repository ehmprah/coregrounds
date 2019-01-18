/**
 * @file
 * Manage modification pages and unlock new ones
 */
import { submitHandler } from '../Util/DOM';
import { request } from '../API';
import Analytics from '../Analytics';
import Config from '../Config/Config';
import Icons from '../Screens/Elements/Icons';
import Messages from '../Screens/Elements/Messages';
import Session from '../Session/Session';
import Sound from '../Sound';

// We keep track of a few things when editing a modification page
let page;

/**
 * Modification page overview and unlock screen
 */
Screens.add('modifications', () => {
  // Build list of available pages
  let html = '';
  const account = Session.getAccount();
  Object.keys(account.modifications.pages).forEach((pageId) => {
    html += `
      <li><button class="box fit" onclick="Screens.show('edit-mod-page', 0, '${pageId}')">
        ${account.modifications.pages[pageId].title}
      </button></li>
    `;
  });
  // Build HTML
  Screens.update('modifications', `
    <div class="container-full">
      <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content">
        <h2 class="text-center first">Select page</h2>
        <ul class="content-row-1">${html}</ul>
        <h2 class="text-center first">Or</h2>
        <button class="box unlock-btn cta fit" id="add-mod-page" onclick="btnUnlockPage()">
          <span>UNLOCK PAGE</span>
          <currency class="credits">
            ${Icons.get('credits')}
            <amount>1000</amount>
          </currency>
        </button>
      </div>
    </div>
  `);
});

/**
 * Modification page edit screen
 */
Screens.add('edit-mod-page', (pageId) => {
  const account = Session.getAccount();
  if (Session.isOnboarding()) {
    const first = account.modifications.pages[Object.keys(account.modifications.pages)[0]];
    page = parseModificationPage(first);
    // Now we prefill the page with the previously unlocked modifications
    Config.get('modifications').forEach((item) => {
      if (account.unlocks.indexOf(item.id) > -1) {
        page.mods[item.group] = new Array(5);
        page.mods[item.group].fill(item.id);
      }
    });
  } else {
    // Find the page in the account's mod pages and parse it
    Object.keys(account.modifications.pages).forEach((id) => {
      if (id === pageId) {
        page = parseModificationPage(account.modifications.pages[pageId]);
      }
    });
  }
  // Update the screen with HTML; we don't do this directly here, because we
  // want to update the screen later when mods have changed without re-parsing
  // the whole page again.
  updateScreen(Session.isOnboarding());
});

function updateScreen(onboarding) {
  // Build HTML for the slots
  const html = ['', '', ''];
  let t;
  let m;
  let mod;
  for (t = 0; t < 3; t++) {
    for (m = 0; m < 5; m++) {
      mod = false;
      if (page.mods[t][m]) {
        mod = Config.query('modifications', { id: page.mods[t][m] });
      }
      // Build html for the button
      html[t] += `
        <button class="box slot" type="button" onclick="modSlotSelect(${t},${m})">
          ${mod ? `<div class="clr-text">${mod.target}</div>${mod.name}` : 'Empty'}
        </button>
      `;
    }
  }

  // Build the screen
  Screens.update('edit-mod-page', `
    <div class="container-full">
      <div class="content">
        <form id="mod-page-edit">
          <div class="content-row-1">
            <h3 class="first">Page title</h3>
            <input type="text" value="${page.title}" id="mod-page-title">
          </div>
          <ul class="content-row-3">
            <li>
              <div class="fit-width">
                <h3 class="first">Offense</h3>
                <div class="wrap">${html[0]}</div>
              </div>
            </li>
            <li>
              <div class="fit-width">
                <h3 class="first">Defense</h3>
                <div class="wrap">${html[1]}</div>
              </div>
            </li>
            <li>
              <div class="fit-width">
                <h3 class="first">Utility</h3>
                <div class="wrap">${html[2]}</div>
              </div>
            </li>
          </ul>
          <div class="form-actions" style="margin: 1em 0;">
            <button class="box" type="submit" id="mod-page-save">Save and continue</button>
            <input type="submit" style="visibility: hidden; position: absolute;" />
            ${onboarding ? '' : `<button class="box grey" type="button" onclick="Screens.show('modifications')">Cancel</button>`}
          </div>
        </form>
      </div>
    </div>
  `);

  // Add submit handler for the save button
  submitHandler('mod-page-save', false, () => {
    // Get the updated page title
    page.title = $('#mod-page-title').value;
    // Save settings to the account
    request('app/save-modifications', {
      page: JSON.stringify(page),
    }, (response) => {
      Messages.show(response.message);
      Sound.play('toggle_interface');
      // We don't need to update the data via the server, we do it locally
      const account = Session.getAccount();
      account.modifications.pages[page.pid] = JSON.parse(JSON.stringify(page));
      Session.setAccount(account);
      if (onboarding) {
        Screens.show('home');
        Screens.show('onboarding-home', 1);
      } else {
        Screens.show('modifications');
      }
    });
  });
}

window.modSlotSelect = function modSlotSelect(group, slot) {
  const account = Session.getAccount();
  const mods = [];

  Config.get('modifications').forEach((mod) => {
    if (group === mod.group && account.unlocks.indexOf(mod.id) >= 0) {
      mods.push({ id: mod.id, title: `${mod.name} [${mod.target}]` });
    }
  });

  Screens.show('select-overlay', 1, mods, (id) => {
    page.title = $('#mod-page-title').value;
    page.mods[group][slot] = id;
    updateScreen();
  });

  return false;
};

/**
 * Send an unlock request for an additional mod page
 */
window.btnUnlockPage = function btnUnlockPage() {
  // Show error and stop before the request if we have insufficient credits
  if (Session.credits() < 1000) {
    Messages.show('Insufficient credits.', 'warning');
    Sound.play('error');
    return;
  }
  // Add throbber class to button
  $(`#add-mod-page`).classList.add('waiting');
  // Send request to webserver
  request('app/unlock-page', {}, (response) => {
    Analytics.event('Unlocks', 'unlock', 'modification page', 0);
    Messages.show(response.message);
    Sound.play('win');
    Screens.refresh();
  });
};

function parseModificationPage(p) {
  const parsed = {
    pid: p.pid,
    title: p.title,
    mods: [new Array(5), new Array(5), new Array(5)],
  };
  if (p.mods.length) {
    parsed.mods = JSON.parse(JSON.stringify(p.mods));
  }
  return parsed;
}
