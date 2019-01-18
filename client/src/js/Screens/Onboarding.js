/**
 * @file
 * The onboarding pages
 */
import { request } from '../API';
import { gameGuideBtn } from '../Screens/Unlock';
import Messages from '../Screens/Elements/Messages';
import Session from '../Session/Session';
import Sound from '../Sound';
import Icons from './Elements/Icons';
import Config from '../Config/Config';
import Sprites from '../Graphics/Sprites';


Screens.add('choose-region', () => {
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
          id="region-${region}"${region === 'us' ? ' checked' : ''}>
        <label for="region-${region}">${regionNames[region]}</label>
      </div>
    `;
  });

  Screens.update('choose-region', `
    <div class="container-flex">
      <div class="content">
        <div style="margin: 1em 0 .5em 0; color: #888;">Choose your server region</div>
        <form id="region-form" autocomplete="off">
          ${radios}
          <button type="submit" class="box wide" style="margin-top:1em;">CONTINUE</button>
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
    const crossregion = 1;
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
      Screens.show('tutorial');
    }, () => {
      $('#region-form button').classList.remove('waiting');
    });
  });
});

Screens.add('onboarding-meta', () => {
  Screens.update('onboarding-meta', `
    <div class="container-flex">
      <div class="blackbox overlaybox">
        <p>
          In Coregrounds you unlock new units and modifications with CREDITS, which you earn by
          playing games. To get you started, we've granted you 10500 FREE CREDITS so you can unlock
          your first units, abilities and modifications.
        </p>
        <p>
          Units you unlock you can use in game (and there's a couple of free ones every day so you
          can try them all). Modifications are bonuses for your units, and you can create
          multiple modification pages for different strategies.
        </p>
        <div class="flex">
          <button class="box wide" onclick="Screens.show('onboarding-towers')">
            UNLOCK FREE UNITS AND MODIFICATIONS
          </button>
        </div>
      </div>
    </div>
  `);
});

Screens.add('onboarding-towers', () => {
  let items = '';
  Config.get('factories').forEach((item) => {
    if (item.tower && item.unlockable) {
      items += `
        <li>
          <unlock class="blackbox" id="unlock-${item.id}">
            ${item.recommended ? '<label class="info black-alpha">recommended</label>' : ''}
            <label>${item.name}</label>
            ${Sprites.withBase(item.id)}
            <button
              class="box unlock-btn vertical"
              data-price="${item.price}"
              onclick="
                btnUnlock(${item.id}, () => {
                  Screens.show('onboarding-minions')
                });
              ">
              <currency class="credits">
                ${Icons.get('credits')}
                <amount>${item.price}</amount>
              </currency>
            </button>
            ${gameGuideBtn(item.id)}
          </unlock>
        </li>
      `;
    }
  });

  Screens.update('onboarding-towers', `
    <div class="container-full">
      <div class="content">
        <ul class="unlocks content-row-4">${items}</ul>
      </div>
    </div>
  `);
});

Screens.add('onboarding-minions', () => {
  let items = '';
  Config.get('factories').forEach((item) => {
    if (item.minion && item.unlockable) {
      items += `
        <li>
          <unlock class="blackbox" id="unlock-${item.id}">
            ${item.recommended ? '<label class="info black-alpha">recommended</label>' : ''}
            <label>${item.name}</label>
            ${Sprites.withBase(item.id)}
            <button
              class="box unlock-btn vertical"
              data-price="${item.price}"
              onclick="
                btnUnlock(${item.id}, () => {
                  Screens.show('onboarding-abilities')
                });
              ">
              <currency class="credits">
                ${Icons.get('credits')}
                <amount>${item.price}</amount>
              </currency>
            </button>
            ${gameGuideBtn(item.id)}
          </unlock>
        </li>
      `;
    }
  });

  Screens.update('onboarding-minions', `
    <div class="container-full">
      <div class="content">
        <ul class="unlocks content-row-4">${items}</ul>
      </div>
    </div>
  `);
});

Screens.add('onboarding-abilities', () => {
  let items = '';
  Config.get('abilities').forEach((item) => {
    if (item.unlockable) {
      items += `
        <li>
          <unlock class="blackbox" id="unlock-${item.id}">
            ${item.recommended ? '<label class="info black-alpha">recommended</label>' : ''}
            <label>${item.name}</label>
            ${Sprites.withBase(item.id)}
            <button
              class="box unlock-btn vertical"
              data-price="${item.price}"
              onclick="
                btnUnlock(${item.id}, () => {
                  Screens.show('onboarding-mod-offensive')
                });
              ">
              <currency class="credits">
                ${Icons.get('credits')}
                <amount>${item.price}</amount>
              </currency>
            </button>
            ${gameGuideBtn(item.id)}
          </unlock>
        </li>
      `;
    }
  });

  Screens.update('onboarding-abilities', `
    <div class="container-full">
      <div class="content">
        <ul class="unlocks content-row-4">${items}</ul>
      </div>
    </div>
  `);
});

Screens.add('onboarding-mod-offensive', () => {
  let items = '';
  Config.get('modifications').forEach((item) => {
    if (item.unlockable && item.group === 0) {
      items += `
        <li>
          <unlock class="blackbox" id="unlock-${item.id}">
            ${item.recommended ? '<label class="info black-alpha">recommended</label>' : ''}
            <div class="flex" style="min-height: 180px;">
              <div class="text">${item.name}</div>
            </div>
            <label class="clr-text">${item.target}</label>
            <label style="color: #888; margin-bottom: 1em;">OFFENSIVE</label>
            <button
              class="box unlock-btn vertical"
              data-price="${item.price}"
              onclick="
                btnUnlock(${item.id}, () => {
                  Screens.show('onboarding-mod-defensive')
                });
              ">
              <currency class="credits">
                ${Icons.get('credits')}
                <amount>${item.price}</amount>
              </currency>
            </button>
          </unlock>
        </li>
      `;
    }
  });
  Screens.update('onboarding-mod-offensive', `
    <div class="container-full">
      <div class="content">
        <ul class="unlocks content-row-4">${items}</ul>
      </div>
    </div>
  `);
});

Screens.add('onboarding-mod-defensive', () => {
  let items = '';
  Config.get('modifications').forEach((item) => {
    if (item.unlockable && item.group === 1) {
      items += `
        <li>
          <unlock class="blackbox" id="unlock-${item.id}">
            ${item.recommended ? '<label class="info black-alpha">recommended</label>' : ''}
            <div class="flex" style="min-height: 180px;">
              <div class="text">${item.name}</div>
            </div>
            <label class="clr-text">${item.target}</label>
            <label style="color: #888; margin-bottom: 1em;">DEFENSIVE</label>
            <button
              class="box unlock-btn vertical"
              data-price="${item.price}"
              onclick="
                btnUnlock(${item.id}, () => {
                  Screens.show('onboarding-mod-utility')
                });
              ">
              <currency class="credits">
                ${Icons.get('credits')}
                <amount>${item.price}</amount>
              </currency>
            </button>
          </unlock>
        </li>
      `;
    }
  });
  Screens.update('onboarding-mod-defensive', `
    <div class="container-full">
      <div class="content">
        <ul class="unlocks content-row-4">${items}</ul>
      </div>
    </div>
  `);
});

Screens.add('onboarding-mod-utility', () => {
  const account = Session.getAccount();
  const first = Object.keys(account.modifications.pages)[0];
  let items = '';
  Config.get('modifications').forEach((item) => {
    if (item.unlockable && item.group === 2) {
      items += `
        <li>
          <unlock class="blackbox" id="unlock-${item.id}">
            ${item.recommended ? '<label class="info black-alpha">recommended</label>' : ''}
            <div class="flex" style="min-height: 180px;">
              <div class="text">${item.name}</div>
            </div>
            <label class="clr-text">${item.target}</label>
            <label style="color: #888; margin-bottom: 1em;">UTILITY</label>
            <button
              class="box unlock-btn vertical"
              data-price="${item.price}"
              onclick="
                btnUnlock(${item.id}, () => {
                  Screens.show('edit-mod-page', 0, ${first.pid});
                  Screens.show('onboarding-mod-page-explanation', 1);
                });
              ">
              <currency class="credits">
                ${Icons.get('credits')}
                <amount>${item.price}</amount>
              </currency>
            </button>
          </unlock>
        </li>
      `;
    }
  });
  Screens.update('onboarding-mod-utility', `
    <div class="container-full">
      <div class="content">
        <ul class="unlocks content-row-4">${items}</ul>
      </div>
    </div>
  `);
});

Screens.add('onboarding-mod-page-explanation', () => {
  Screens.update('onboarding-mod-page-explanation', `
    <div class="container-full bg-contrast flex" onclick="window.event.stopPropagation();">
      <div class="blackbox overlaybox">
        <p>
          Great! This is your first modification page. Each modification can be used up to five
          times in one modification page. You can give it a name now, if you like. Then save the
          page to continue!
        </p>
        <button class="box wide" onclick="Screens.hide(1);">
          CONTINUE
        </button>
      </div>
    </div>
  `);
});

Screens.add('onboarding-home', () => {
  Session.finishOnboarding();
  Screens.update('onboarding-home', `
    <div class="container-full bg-contrast flex" onclick="window.event.stopPropagation();">
      <div class="blackbox overlaybox">
        <p>
          Aaaaand you're done and ready for your first match! If you run into trouble anywhere,
          join us on Discord or Reddit or send us an email, we're happy to help.
        </p>
        <p>
          Coregrounds is made by a tiny indie team and we appreciate your support a
          lot! If you like the game, tell your friends about it, write a nice review or purchase
          some cordium to help us pay the server costs!
        </p>
        <p>And most important of all: go and have fun on the Coregrounds now!</p>
        <button class="box wide" onclick="Screens.hide(1);">
          CONTINUE
        </button>
      </div>
    </div>
  `);
});
