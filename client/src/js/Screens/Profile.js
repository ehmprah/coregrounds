/**
 * @file
 * Template for a player's profile page (defaults to current player)
 */
import Config from '../Config/Config';
import Session from '../Session/Session';
import { request } from '../API';
import Icons from '../Screens/Elements/Icons';

Screens.add('profile', (uid) => {
  if (uid === null) {
    const account = Session.getAccount();
    updateProfile(account);
  } else {
    Screens.update('profile', `<throbber>LOADING</throbber>`);
    request(`app/profile/${uid}`, false, (response) => {
      updateProfile(response.profile);
    });
  }
});

function updateProfile(account) {
  const achievements = `${(parseFloat(account.achievementProgress) * 100).toFixed(2)}%`;
  const icon = Config.query('playerIcons', { id: account.icon });
  // Calculate winrates
  const winrate = {};
  ['season', 'previous'].forEach((key) => {
    winrate[key] = {};
    ['bot', 'casual', 'ranked'].forEach((type) => {
      winrate[key][type] = 0;
      if (account[key][type].games) {
        winrate[key][type] =
          ((account[key][type].wins / account[key][type].games) * 100).toFixed(1);
      }
    });
  });

  Screens.update('profile', `
    <div class="container flex">
      <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content">
        <profile-data>
          <ul class="flex-row-3">
            <li class="blackbox">
              <img src="player-icons/${icon.file}">
              <label style="margin: .5em;">${account.name}</label>
            </li>
            <li class="blackbox">
              <label>RANK</label>
              <span class="huge">${Math.floor(account.season.ranked.rank / 5)}</span>
              <div class="clearfix rank rank-${account.season.ranked.rank % 5}">
                <div></div><div></div><div></div><div></div><div></div>
              </div>
            </li>
            <li class="blackbox">
              <label>WINSTREAK</label>
              <span class="huge">${account.winStreak}</span>
              <label>PVP MATCHES</label>
            </li>
          </ul>
          <ul class="flex-row-3">
            <li class="blackbox">
              <label class="season">CURRENT SEASON</label>
              <label>BOT WINS</label>
              <span class="huge">${account.season.bot.wins}</span>
              <label>W/L ${winrate.season.bot}%</label>
            </li>
            <li class="blackbox">
              <label class="season">CURRENT SEASON</label>
              <label>CASUAL WINS</label>
              <span class="huge">${account.season.casual.wins}</span>
              <label>W/L ${winrate.season.casual}%</label>
            </li>
            <li class="blackbox">
              <label class="season">CURRENT SEASON</label>
              <label>RANKED WINS</label>
              <span class="huge">${account.season.ranked.wins}</span>
              <label>W/L ${winrate.season.ranked}%</label>
            </li>
          </ul>
          <ul class="flex-row-3">
            <li class="blackbox">
              <label class="season">PREVIOUS SEASONS</label>
              <label>BOT WINS</label>
              <span class="huge">${account.previous.bot.wins}</span>
              <label>W/L ${winrate.previous.bot}%</label>
            </li>
            <li class="blackbox">
              <label class="season">PREVIOUS SEASONS</label>
              <label>CASUAL WINS</label>
              <span class="huge">${account.previous.casual.wins}</span>
              <label>W/L ${winrate.previous.casual}%</label>
            </li>
            <li class="blackbox">
              <label class="season">PREVIOUS SEASONS</label>
              <label>RANKED WINS</label>
              <span class="huge">${account.previous.ranked.wins}</span>
              <label>W/L ${winrate.previous.ranked}%</label>
            </li>
          </ul>
          <ul class="flex-row-4">
            <li class="blackbox" style="justify-content: space-between;">
              <label>LEVEL</label>
              <progress-bar>
                <bar style="width: ${account.level.progress * 100}%;"></bar>
                <span>Level ${account.level.current}</span>
              </progress-bar>
            </li>
            <li class="blackbox">
              <label>CREDITS EARNED</label>
              <span class="huge">${account.credits.earned}</span>
            </li>
            <li class="blackbox">
              <label>ITEMS UNLOCKED</label>
              <span class="huge">${account.unlocks.length - 4}</span>
            </li>
            <li class="blackbox" style="justify-content: space-between;">
              <label>ACHIEVEMENTS</label>
              <progress-bar>
                <bar style="width:${achievements}"></bar>
                <span>${achievements}</span>
              </progress-bar>
            </li>
          </ul>
        </profile-data>
      </div>
    </div>
  `);
}
