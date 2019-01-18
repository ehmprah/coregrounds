/**
 * @file
 * Settings screens
 */
import { buildTabs, submitHandler, validateEmail } from '../Util/DOM';
import { request } from '../API';
import Analytics from '../Analytics';
import Config from '../Config/Config';
import Event from '../Event';
import Game from '../Game/Game';
import Messages from '../Screens/Elements/Messages';
import GameServer from '../Network/GameServer';
import Session from '../Session/Session';
import Sprites from '../Graphics/Sprites';
import Sound from '../Sound';
import Icons from '../Screens/Elements/Icons';

const screens = {
  'settings-customize': 'Customize',
  'settings-game': 'Game',
  'settings-account': 'Account',
  'settings-region': 'Region',
};

// We keep track of the current classes on the body so we can change those
// on the fly in the customize settings and can revert to those on cancel
let bodyClasses;

/**
 * Add customize settings screen
 */
Screens.add('settings-customize', () => {
  const account = Session.getAccount();
  let skins = '';
  let colors = '';
  let icons = '';
  let backgrounds = '';

  // First, we get all unlocked skins, grouped by unit
  const units = {};
  Config.get('skins').forEach((skin) => {
    if (account.unlocks.indexOf(skin.id) >= 0) {
      // Create an array of skins per unit
      if (units[skin.unitId] === undefined) {
        units[skin.unitId] = {
          skins: [],
          selected: 0,
        };
      }
      units[skin.unitId].skins.push(skin);
      // Check for selected skins
      if (account.skins.indexOf(skin.id) >= 0) {
        units[skin.unitId].selected = skin.id;
      }
    }
  });

  // Then we build the selection per unit
  let options;
  Object.keys(units).forEach((id) => {
    options = `
      <input class="skin" type="radio" id="unit-${id}-0" name="unit-${id}" value="0"${units[id].selected === 0 ? ' checked' : ''}>
      <label class="clr-text" for="unit-${id}-0">
        ${Sprites.withBase(id)}
      </label>
    `;
    units[id].skins.forEach((skin) => {
      options += `
        <input class="skin" type="radio" id="unit-${id}-${skin.id}" name="unit-${id}" value="${skin.id}"${units[id].selected === skin.id ? ' checked' : ''}>
        <label class="clr-text" for="unit-${id}-${skin.id}">
          ${Sprites.withBase(skin.id)}
        </label>
      `;
    });
    skins += `
      <div class="form-item image-select">${options}</div>
    `;
  });

  if (!skins.length) {
    skins += `
      <p>You don't have any skins yet.</p>
      <button class="box" onclick="Screens.show('store-skins')">
        Get skins
      </button>
    `;
  }

  // Add colors
  Config.get('colors').forEach((color) => {
    if (account.unlocks.indexOf(color.id) >= 0) {
      colors += `
        <input type="radio" id="color-${color.id}" name="color" value="${color.id}"${color.id === account.color ? ' checked' : ''}>
        <label class="clr-text" for="color-${color.id}" onclick="setBodyClass('color', ${color.id})">
          ${Sprites.withBase(1103, color.main)}
        </label>
      `;
    }
  });

  // Add playerIcons
  Config.get('playerIcons').forEach((icon) => {
    if (account.unlocks.indexOf(icon.id) >= 0) {
      icons += `
        <input type="radio" id="icon-${icon.id}" name="icon" value="${icon.id}"${icon.id === account.icon ? ' checked' : ''}>
        <label class="clr-text" for="icon-${icon.id}">
          <img src="player-icons/${icon.file}">
        </label>
      `;
    }
  });

  // Add backgrounds
  backgrounds += `
    <input type="radio" id="background-0" name="background" value="0"${account.background === 0 ? ' checked' : ''}>
    <label class="clr-text" for="background-0" onclick="setBodyClass('background', 0)">
      <div class="background-0 fit"></div>
    </label>
  `;
  Config.get('backgrounds').forEach((background) => {
    if (account.unlocks.indexOf(background.id) >= 0) {
      backgrounds += `
        <input type="radio" id="background-${background.id}" name="background" value="${background.id}"${background.id === account.background ? ' checked' : ''}>
        <label class="clr-text" for="background-${background.id}" onclick="setBodyClass('background', ${background.id})">
          <div class="background-${background.id} fit"></div>
        </label>
      `;
    }
  });

  // Build screen
  Screens.update('settings-customize', `
    <div class="container-full">
      <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content">
        ${buildTabs(screens, 'settings-customize')}
        <form id="settings-appearance-form">
          <h3 class="first">Wall skins</h3>
          <div class="form-item image-select">${skins}</div>
          <h3 class="first">Color</h3>
          <div class="form-item image-select">${colors}</div>
          <h3>Player Icon</h3>
          <div class="form-item image-select">${icons}</div>
          <h3>Background</h3>
          <div class="form-item image-select">${backgrounds}</div>
          <div class="form-actions">
            <button id="settings-submit" class="box">Save settings</button>
          </div>
        </form>
      </div>
    </div>
  `);

  // Add submit handler for the save button
  submitHandler('settings-submit', false, () => {
    // Get all selected skin ids
    const selected = [];
    $$('input.skin:checked').forEach((input) => {
      const id = parseInt(input.value, 10);
      if (id) {
        selected.push(id);
      }
    });
    // Reset the bodyClasses and keep the changes
    bodyClasses = null;
    // Build post data for the request
    const data = {
      colorId: parseInt($('input[name="color"]:checked').value, 10),
      playerIcon: parseInt($('input[name="icon"]:checked').value, 10),
      background: parseInt($('input[name="background"]:checked').value, 10),
      skins: JSON.stringify(selected),
    };
    // Post request against the web server
    request('app/customize', data, (response) => {
      // Update the local copy of the account data accordingly
      account.color = data.colorId;
      account.icon = data.playerIcon;
      account.background = data.background;
      account.skins = JSON.parse(JSON.stringify(selected));
      Session.setAccount(account);
      // Show success message
      Messages.show(response.message);
      Sound.play('toggle_interface');
      $('#settings-submit').classList.remove('waiting');
    }, () => {
      $('#settings-submit').classList.remove('waiting');
    });
  });
});

Event.on('showing settings-customize', () => {
  bodyClasses = $('body').className;
});

Event.on('leaving settings-customize', () => {
  if (bodyClasses) {
    $('body').className = bodyClasses;
  }
});

// TODO remove this!
window.setBodyClass = function setBodyClass(type, id) {
  // Remove old color class (we have to loop here because we don't know the id)
  document.body.classList.forEach((cls) => {
    if (cls.indexOf(type) >= 0) {
      document.body.classList.remove(cls);
    }
  });

  // And add the new color class accordingly
  document.body.classList.add(`${type}-${id}`);
};

/**
 * Add sound settings screen
 */
Screens.add('settings-game', () => {
  const music = Config.User.get('music-volume', 1);
  const sound = Config.User.get('sound-volume', 1);
  Screens.update('settings-game', `
    <div class="container-full">
      <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content">
        ${buildTabs(screens, 'settings-game')}
        <form>
          <h3>Music & Sound</h3>
          <div class="form-item form-range">
            <label for="sound-volume">Sound Volume<span>${Math.round(sound * 100)}%</span></label>
            <input type="range" value="${sound * 100}" step="5" min="0" max="100" id="sound-volume">
          </div>
          <div class="form-item form-range">
            <label for="music-volume">Music Volume<span>${Math.round(music * 100)}%</span></label>
            <input type="range" value="${music * 100}" step="5" min="0" max="100" id="music-volume">
          </div>
          <div class="form-actions">
            <button id="settings-submit" class="box">Save settings</button>
            <button type="button" class="box" onclick="toggleFullscreen();">Toggle fullscreen</button>
          </div>
        </form>
      </div>
    </div>
  `);

  // Add listeners for volume slider percentage label update
  $('#music-volume').addEventListener('input', (e) => {
    $('label[for="music-volume"] span').innerHTML = `${e.currentTarget.value}%`;
  });
  $('#sound-volume').addEventListener('input', (e) => {
    $('label[for="sound-volume"] span').innerHTML = `${e.currentTarget.value}%`;
  });

  // Add submit handler for the save button
  submitHandler('settings-submit', false, () => {
    // Save settings to local storage
    Config.User.set('music-volume', $('#music-volume').value / 100);
    Config.User.set('sound-volume', $('#sound-volume').value / 100);
    // Adjust volumes
    Event.fire('music volume changed');
    Event.fire('sound volume changed');
    // Display success message
    Messages.show('Settings saved!');
    Sound.play('toggle_interface');
    // Remove waiting class from button
    $('#settings-submit').classList.remove('waiting');
  }, () => {
    $('#settings-submit').classList.remove('waiting');
  });
});

/**
 * Add account settings screen
 */
Screens.add('settings-account', () => {
  const account = Session.getAccount();
  if (account.validated) {
    Screens.update('settings-account', `
      <div class="container-full">
        <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
        <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
        <div class="content">
          ${buildTabs(screens, 'settings-account')}
          <div class="blackbox" style="padding: .75em;">
            If you have a coupon code, you can redeem it
            <a style="font-weight:bold;" href="#" onclick="Screens.show('redeem');">here.</a>
          </div>
          <h3>Your Coregrounds account</h3>
          <p style="color: #888;">
            Your account is linked to a Coregrounds account with the email address:
          </p>
          <p>
            <span>${account.mail}</span>
            <a href="${Config.ENDPOINT}/player">[change]</a>
          </p>
          <form id="subscribe-form" autocomplete="off" style="margin: 2em 0;">
            <h3>Newsletter subscription</h3>
            <p style="color: #888;">
              While being subscribed to the newsletter, you get a 50% credit bonus!
            </p>
            <div style="margin-bottom: 1em;">
              <input type="checkbox" id="subscribe" ${account.newsletter ? 'checked' : ''}>
              <label for="subscribe">SUBSCRIBED</label>
            </div>
            <button type="submit" class="box" id="subscribe-submit">Save settings</button>
            <input type="submit" style="visibility: hidden; position: absolute;" />
          </form>
        </div>
      </div>
    `);
    // Attach submit handlers
    submitHandler('subscribe-submit', false, () => {
      const subscribe = $('#subscribe').checked ? 1 : 0;
      request('app/subscribe', {
        subscribe,
      }, (response) => {
        if (account.newsletter === 1 && subscribe === 0) {
          account.bonus.permanent -= 50;
        }
        if (account.newsletter === 0 && subscribe === 1) {
          account.bonus.permanent += 50;
        }
        account.newsletter = subscribe;
        Session.setAccount(account);
        Messages.show(response.message);
        Sound.play('toggle_interface');
        $('#subscribe-submit').classList.remove('waiting');
      }, () => {
        $('#subscribe-submit').classList.remove('waiting');
      });
      return false;
    });
  } else {
    Screens.update('settings-account', `
      <div class="container-full">
        <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
        <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
        <div class="content">
          ${buildTabs(screens, 'settings-account')}
          <div class="blackbox" style="padding: .75em;">
            If you have a coupon code, you can redeem it
            <a style="font-weight:bold;" href="#" onclick="Screens.show('redeem');">here.</a>
          </div>
          <h3>Link Coregrounds account</h3>
          <p>If you want to play Coregrounds on other platforms than Steam, you need to link
          your account to a Coregrounds account. With a linked account you can also
          subscribe to the newsletter, which will grant you a 50% credit bonus!</p>
          <form id="link-form" autocomplete="off" style="max-width: 320px;">
            <input type="text" id="link-mail" class="margin-bottom" placeholder="Your email address" />
            <button type="submit" class="box" id="link-submit" style="margin-top: 1em;">
              Link account
            </button>
            <input type="submit" style="visibility: hidden; position: absolute;" />
          </form>
        </div>
      </div>
    `);
    // Attach submit handlers
    submitHandler('link-submit', () => {
      // Check if we have input in the mail field
      if (!$('#link-mail').value.length) {
        Messages.show('Please enter your email address.', 'error');
        Sound.play('error');
        return false;
      }
      // Check if we have a valid email address
      if (!validateEmail($('#link-mail').value)) {
        Messages.show('Please enter a valid email address.', 'error');
        Sound.play('error');
        return false;
      }
      return true;
    }, () => {
      // Post request against the web server
      request('app/upgrade', {
        mail: $('#link-mail').value,
      }, (response) => {
        Messages.show(response.message);
        Sound.play('toggle_interface');
        $('#link-submit').classList.remove('waiting');
      }, () => {
        $('#link-submit').classList.remove('waiting');
      });
      return false;
    });
  }
});

Screens.add('settings-region', () => {
  const account = Session.getAccount();
  const regionNames = {
    us: 'America',
    europe: 'Europe',
    asia: 'Asia',
  };
  let radios = '';
  ['us', 'europe', 'asia'].forEach((region) => {
    radios += `
      <div>
        <input type="radio" name="region" value="${region}"
          id="region-${region}"${account.region === region ? ' checked' : ''}>
        <label for="region-${region}">${regionNames[region]}</label>
      </div>
    `;
  });

  Screens.update('settings-region', `
    <div class="container-full">
      <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content">
        ${buildTabs(screens, 'settings-region')}
        <h3>Region</h3>
        <div style="margin-top: 1em; color: #888;">Choose your server region:</div>
        <form id="region-form" autocomplete="off">
          ${radios}
          <div style="margin-top: 1em; color: #888;">
            Optional: allow pvp games with players from other regions?
          </div>
          <div style="margin-bottom: 1em;">
            <input type="checkbox" id="crossregion" ${account.crossregion ? 'checked' : ''}>
            <label for="crossregion">Cross-regional matchmaking</label>
          </div>
          <button type="submit" class="box">Save region settings</button>
          <input type="submit" style="visibility: hidden; position: absolute;" />
        </form>
      </div>
    </div>
  `);

  const form = document.getElementById('region-form');
  form.addEventListener('submit', (e) => {
    e.preventDefault();
    const data = new FormData(form);
    const region = data.get('region');
    const crossregion = $('#crossregion').checked ? 1 : 0;
    request('app/set-region', {
      region,
      crossregion,
    }, () => {
      account.region = region;
      account.crossregion = crossregion;
      Session.setAccount(account);
      Messages.show('Region settings saved.');
      Sound.play('toggle_interface');
      $('#region-form button').classList.remove('waiting');
    }, () => {
      $('#region-form button').classList.remove('waiting');
    });
  });
});

Screens.add('settings-ingame', () => {
  const music = Config.User.get('music-volume');
  const sound = Config.User.get('sound-volume');

  // Add surrender option if applicable
  let surrender = '';
  if (Game.phase() === 2) {
    surrender = `
      <h3 class="first">Surrender</h3>
      <button type="button" class="box red" onclick="Screens.show('surrender-confirm', 1)">Surrender</button>
    `;
  }

  Screens.update('settings-ingame', `
    <div class="container-full bg-contrast flex" onclick="Screens.hide(1)">
      <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="settings-ingame blackbox" onclick="window.event.stopPropagation();">
        <form>
          ${surrender}
          <h3 class="first">Settings</h3>
          <button type="button" class="box" onclick="toggleFullscreen();">Toggle fullscreen</button>
          <div class="form-item form-range">
            <label for="sound-volume">Sound Volume<span>${Math.round(sound * 100)}%</span></label>
            <input type="range" value="${sound * 100}" step="5" min="0" max="100" id="sound-volume">
          </div>
          <div class="form-item form-range">
            <label for="music-volume">Music Volume<span>${Math.round(music * 100)}%</span></label>
            <input type="range" value="${music * 100}" step="5" min="0" max="100" id="music-volume">
          </div>
          <button type="button" class="box" id="settings-submit">Save settings</button>
          <button type="button" class="box grey" onclick="Screens.hide(1)">back</button>
        </form>
      </div>
    </div>
  `);

  // Add listeners for volume slider percentage label update
  $('#music-volume').addEventListener('input', (e) => {
    $('label[for="music-volume"] span').innerHTML = `${e.currentTarget.value}%`;
  });
  $('#sound-volume').addEventListener('input', (e) => {
    $('label[for="sound-volume"] span').innerHTML = `${e.currentTarget.value}%`;
  });

  // Add submit handler for the save button
  submitHandler('settings-submit', false, () => {
    // Save settings to local storage
    Config.User.set('music-volume', $('#music-volume').value / 100);
    Config.User.set('sound-volume', $('#sound-volume').value / 100);
    // Adjust volumes
    Event.fire('music volume changed');
    Event.fire('sound volume changed');
    // Display success message
    Messages.show('Settings saved!');
    Sound.play('toggle_interface');
    // Remove waiting class from button
    $('#settings-submit').classList.remove('waiting');
  }, () => {
    $('#settings-submit').classList.remove('waiting');
  });
});

/**
 * Add surrender confirm screen
 */
Screens.add('surrender-confirm', () => {
  Screens.update('surrender-confirm', `
    <div class="container-full bg-contrast flex">
      <div class="settings-ingame blackbox">
        <h3 class="first">Are you sure?</h3>
        <button id="surrender-submit" class="box red">Surrender</button>
        <button class="box grey" onclick="Screens.hide(1)">back</button>
      </div>
    </div>
  `);
  submitHandler('surrender-submit', false, () => {
    GameServer.emit('Surrender');
    Analytics.event('Game', 'surrender');
    Screens.hide(1);
  }, () => {
    $('#surrender-submit').classList.remove('waiting');
  });
});
