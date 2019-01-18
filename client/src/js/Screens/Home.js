/**
 * @file
 * Template for the home screen
 */
import Event from '../Event';
import Config from '../Config/Config';
import { todayString } from '../Util';
import { request } from '../API';
import Icons from '../Screens/Elements/Icons';
import Queue from '../Screens/Elements/Queue';
import Sprites from '../Graphics/Sprites';
import Session from '../Session/Session';
import Picks from '../Graphics/Elements/Picks';

let data = {};

Event.on('app loaded', () => {
  // Get unique daily identifier and previously saved home screen data
  const today = todayString();
  const raw = window.localStorage.getItem('cgs-home');
  // If we have data, parse the json
  if (raw) data = JSON.parse(raw);
  // Update the free picks
  if (data.data && data.data.free) Picks.updateFreePicks(data.data.free);
  // If we do not have an up-to-date homepage, get it now
  if (data.updated !== today) {
    request('app/home', false, (response) => {
      // Save the HTML in the local storage for the rest of the day
      window.localStorage.setItem('cgs-home', JSON.stringify({
        updated: today,
        data: response.data,
      }));
      // Update the local copy of the data
      data.data = response.data;
      // Update the free picks
      Picks.updateFreePicks(data.data.free);
      // If we're currently on the home screen, update it
      build();
    });
  }
});

// Add screen and build callback
Screens.add('home', build);

function build() {
  if (!data) {
    // We wait until we have free picks, news and video before we build this
    Screens.update('home', `
      <throbber>LOADING HOME</throbber>
    `);
  } else {
    if (!Session.ready()) {
      return setTimeout(build, 100);
    }
    // Get player account
    const account = Session.getAccount();
    // Get player icon
    const icon = Config.query('playerIcons', { id: account.icon });
    // Calculate bonuses
    let total = 0;
    // Add daily bonus and update it on the bonus summary
    total += account.bonus.daily;
    // Add perma bonus and update it on the bonus summary
    total += account.bonus.permanent;
    // Check if we have a bonus per win
    if (account.bonus.wins > 0) total += 100;
    // Check if we have a bonus over time
    const now = Date.now() / 1000;
    if (account.bonus.time > now) total += 100;
    // Build free picks of the day
    let picks = '';
    data.data.free.forEach((id) => {
      picks += Sprites.withBase(id, false, 0);
    });

    // Build play btn
    let playBtn = '';
    if (Queue.active()) {
      playBtn = `
        <button class="box fit" disabled>
          <div class="icon">${Icons.get('wait')}</div>
          <div>IN QUEUE</div>
        </button>
      `;
    } else {
      playBtn = `
        <button class="box fit" onclick="Screens.show('${account.tutorial ? 'play' : 'tutorial'}')">
          <div class="icon">${Icons.get('play')}</div>
          <div>PLAY</div>
        </button>
      `;
    }

    // Build HTML
    Screens.update('home', `
      <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
      <div class="container flex">
        <grid class="flex-child">

          <tile class="tile-play">
            ${playBtn}
          </tile>

          <tile class="currency-summary clickable blackbox" onclick="Screens.show('buy')">
            <currency class="player currency">
              ${Icons.get('currency')}
              <amount>${account.currency.current}</amount>
            </currency>
          </tile>

          <tile class="tile-unlock">
            <button class="box fit" onclick="Screens.show('unlock-towers')">
              <div class="icon">${Icons.get('unlock')}</div>
              <div>UNLOCK</div>
            </button>
          </tile>

          <tile class="credits-summary clickable blackbox" onclick="Screens.show('store-credits')">
            <div class="credits-summary-overview">
              <currency class="player credits">
                ${Icons.get('credits')}
                <amount>${account.credits.current}</amount>
              </currency>
              <currency class="player bonus">
                ${Icons.get('bonus')}
                <amount>${total}</amount>
              </currency>
            </div>
            <div class="credits-summary-details">
              <bonus>
                <div class="bonus-title">Daily</div>
                <div class="bonus-amount">${account.bonus.daily}</div>
              </bonus>
              <bonus>
                <div class="bonus-title">Permanent</div>
                <div class="bonus-amount">${account.bonus.permanent}</div>
              </bonus>
              <bonus>
                <div class="bonus-title">Per win</div>
                <div class="bonus-amount">
                  ${account.bonus.wins > 0 ? 100 : 0}
                </div>
                <div class="bonus-duration">
                  ${account.bonus.wins > 0 ? `${account.bonus.wins} wins` : ''}
                </div>
              </bonus>
              <bonus>
                <div class="bonus-title">Over time</div>
                <div class="bonus-amount">
                  ${account.bonus.time > now > 0 ? 100 : 0}
                </div>
                <div class="bonus-duration">
                  ${account.bonus.time > now ? `${Math.round((account.bonus.time - now) / 3600)} hours` : ''}
                </div>
              </bonus>
            </div>
          </tile>

          <tile class="player-summary clickable blackbox" onclick="Screens.show('profile', 0, null)">
            <div class="player-icon"><img src="player-icons/${icon.file}"></div>
            <div class="player-info">
              <div class="player-name">${account.name}</div>
              <progress-bar>
                <bar style="width: ${account.level.progress * 100}%;"></bar>
                <span>Level ${account.level.current}</span>
              </progress-bar>
            </div>
          </tile>

          <tile class="achievements-summary clickable blackbox" onclick="Screens.show('achievements')">
            <label>ACHIEVEMENTS</label>
            <progress-bar>
              <bar style="width: ${account.achievementProgress * 100}%;"></bar>
              <span>${(account.achievementProgress * 100).toFixed(2)}%</span>
            </progress-bar>
          </tile>

          <tile class="tile-free-picks blackbox">
            <label>FREE PICKS OF THE DAY</label>
            <div class="wrap">${picks}</div>
          </tile>

          <tile class="tile-news blackbox clickable" onclick="Screens.show('news')" style="background-image: url(${data.data.news[0].image})">
            <label>${data.data.news[0].title}</label>
          </tile>

          <tile class="tile-game-guide">
            <button class="box fit" onclick="Screens.show('game-guide')">
              <div class="icon">${Icons.get('help')}</div><div>GAME GUIDE</div>
            </button>
          </tile>

          <tile>
            <button class="box fit" onclick="Screens.show('traininggrounds')">
              <div class="icon">${Icons.get('traininggrounds')}</div>
              <div>TRAINING GROUNDS</div>
            </button>
          </tile>

          <tile class="tile-supporter">
            <button class="box fit" onclick="Screens.show('buy')">
              <div class="icon">${Icons.get('support')}</div>
              <div>INDIE SUPPORTER PACK</div>
            </button>
          </tile>

          <tile>
            <button class="box fit" onclick="window.open('http://discord.gg/xQmYUPr');">
              <svg class="external" fill="#ffffff" height="24" viewBox="0 0 24 24" width="24" xmlns="http://www.w3.org/2000/svg">
                  <path d="M0 0h24v24H0z" fill="none"/>
                  <path d="M19 19H5V5h7V3H5c-1.11 0-2 .9-2 2v14c0 1.1.89 2 2 2h14c1.1 0 2-.9 2-2v-7h-2v7zM14 3v2h3.59l-9.83 9.83 1.41 1.41L19 6.41V10h2V3h-7z"/>
              </svg>
              <div class="icon">
                <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 245 240"><path fill="#FFFFFF" d="M104.4 103.9c-5.7 0-10.2 5-10.2 11.1s4.6 11.1 10.2 11.1c5.7 0 10.2-5 10.2-11.1.1-6.1-4.5-11.1-10.2-11.1zM140.9 103.9c-5.7 0-10.2 5-10.2 11.1s4.6 11.1 10.2 11.1c5.7 0 10.2-5 10.2-11.1s-4.5-11.1-10.2-11.1z"/><path class="st0" d="M189.5 20h-134C44.2 20 35 29.2 35 40.6v135.2c0 11.4 9.2 20.6 20.5 20.6h113.4l-5.3-18.5 12.8 11.9 12.1 11.2 21.5 19V40.6c0-11.4-9.2-20.6-20.5-20.6zm-38.6 130.6s-3.6-4.3-6.6-8.1c13.1-3.7 18.1-11.9 18.1-11.9-4.1 2.7-8 4.6-11.5 5.9-5 2.1-9.8 3.5-14.5 4.3-9.6 1.8-18.4 1.3-25.9-.1-5.7-1.1-10.6-2.7-14.7-4.3-2.3-.9-4.8-2-7.3-3.4-.3-.2-.6-.3-.9-.5-.2-.1-.3-.2-.4-.3-1.8-1-2.8-1.7-2.8-1.7s4.8 8 17.5 11.8c-3 3.8-6.7 8.3-6.7 8.3-22.1-.7-30.5-15.2-30.5-15.2 0-32.2 14.4-58.3 14.4-58.3 14.4-10.8 28.1-10.5 28.1-10.5l1 1.2c-18 5.2-26.3 13.1-26.3 13.1s2.2-1.2 5.9-2.9c10.7-4.7 19.2-6 22.7-6.3.6-.1 1.1-.2 1.7-.2 6.1-.8 13-1 20.2-.2 9.5 1.1 19.7 3.9 30.1 9.6 0 0-7.9-7.5-24.9-12.7l1.4-1.6s13.7-.3 28.1 10.5c0 0 14.4 26.1 14.4 58.3 0 0-8.5 14.5-30.6 15.2z"/></svg>
              </div>
              <div>COMMUNITY</div>
            </button>
          </tile>

          <tile>
            <button class="box fit" onclick="window.open('http://reddit.com/r/coregrounds');">
              <svg class="external" fill="#ffffff" height="24" viewBox="0 0 24 24" width="24" xmlns="http://www.w3.org/2000/svg">
                <path d="M0 0h24v24H0z" fill="none"/>
                <path d="M19 19H5V5h7V3H5c-1.11 0-2 .9-2 2v14c0 1.1.89 2 2 2h14c1.1 0 2-.9 2-2v-7h-2v7zM14 3v2h3.59l-9.83 9.83 1.41 1.41L19 6.41V10h2V3h-7z"/>
              </svg>
              <div class="icon">
                <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 341.8 341.8"><path d="M170.9 30.364A140.536 140.536 0 0 0 30.364 170.9 140.536 140.536 0 0 0 170.9 311.436 140.536 140.536 0 0 0 311.436 170.9 140.536 140.536 0 0 0 170.9 30.364zm58.68 55.557c8.054 0 14.629 6.575 14.629 14.629 0 8.054-6.575 14.629-14.63 14.629-7.889 0-14.3-6.246-14.628-13.972l-30.408-6.41-9.37 43.887c21.369.821 40.764 7.396 54.736 17.423 3.616-3.616 8.547-5.753 14.135-5.753 11.342 0 20.546 9.205 20.546 20.546 0 8.383-5.095 15.615-11.834 18.902.329 1.973.493 3.945.493 6.082 0 31.559-36.654 57.036-82.02 57.036-45.366 0-82.02-25.477-82.02-57.036 0-2.137.164-4.273.493-6.246-7.233-3.287-12.164-10.355-12.164-18.738 0-11.341 9.205-20.546 20.546-20.546 5.425 0 10.52 2.301 14.136 5.753 14.136-10.355 33.696-16.766 55.557-17.423l10.355-48.982c.329-.987.822-1.808 1.644-2.302.822-.493 1.808-.657 2.794-.493l34.025 7.233c2.3-4.931 7.232-8.219 12.985-8.219zM138.684 170.9c-8.054 0-14.63 6.575-14.63 14.629 0 8.054 6.576 14.629 14.63 14.629 8.054 0 14.628-6.575 14.628-14.63 0-8.053-6.574-14.628-14.628-14.628zm64.432 0c-8.054 0-14.629 6.575-14.629 14.629 0 8.054 6.575 14.629 14.63 14.629 8.053 0 14.628-6.575 14.628-14.63 0-8.053-6.575-14.628-14.629-14.628zm-64.021 46.72c-.987 0-1.973.372-2.713 1.111-1.479 1.48-1.479 3.945 0 5.425 9.863 9.862 29.094 10.684 34.682 10.684 5.589 0 24.656-.658 34.682-10.684 1.48-1.48 1.48-3.78.329-5.425-1.48-1.479-3.945-1.479-5.424 0-6.41 6.246-19.724 8.548-29.422 8.548s-23.176-2.302-29.422-8.548a3.82 3.82 0 0 0-2.712-1.11z" fill="#fff"/></svg>
              </div>
              <div>FORUM</div>
            </button>
          </tile>

          <tile class="tile-version">
            <button class="box fit" onclick="Screens.show('changelog-${Config.VERSION}')">
              <div class="icon">${Icons.get('changelog')}</div>
              <div>VERSION ${Config.VERSION}</div>
            </button>
          </tile>

          <tile class="tile-video blackbox clickable" onclick="Screens.show('video', 0, '${data.data.video.id}')" style="background-image:url(http://img.youtube.com/vi/${data.data.video.id}/sddefault.jpg)">
            <div class="video-play">${Icons.get('play')}</div>
          </tile>

        </grid>
      </div>
    `);
  }
}
