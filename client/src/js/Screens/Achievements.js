/**
 * @file Achievements.js
 * Screen showing a player's achievements
 *
 * TODO add caching for the achievement data
 */
import Config from '../Config/Config';
import { request } from '../API';
import Session from '../Session/Session';
import Icons from '../Screens/Elements/Icons';
import Sound from '../Sound';

Screens.add('achievements', (uid) => {
  // If we didn't get a uid, we default to the currently logged in user
  if (!uid) {
    ({ uid } = Session.getAccount());
  }

  // Show throbber before we query for the current achievements
  Screens.update('achievements', `
    <throbber>LOADING ACHIEVEMENTS</throbber>
  `);

  // Send request to update the achievements
  request(`app/achievements/${uid}`, false, (response) => {
    // Build page with the newest data
    build(response);
  });
});

function build(data) {
  const achievements = Config.get('achievements');
  let list = '';
  let progress;

  achievements.forEach((achievement) => {
    // Calculate achievement progress
    progress = data.achievements[achievement.id] >= achievement.threshold
      ? '100.00%'
      : `${((data.achievements[achievement.id] / achievement.threshold) * 100).toFixed(2)}%`;
    // Build HTML
    list += `
      <li class="blackbox${progress === '100.00%' ? ' completed' : ''}">
        <header>
          <label>${achievement.title}</label>
          <currency class="credits inline">
            ${Icons.get('credits')}
            <amount>${achievement.credits}</amount>
          </currency>
        </header>
        <progress-bar>
          <bar style="width:${progress}"></bar>
          <span>${progress}</span>
        </progress-bar>
        <description>${achievement.description}</description>
      </li>
    `;
  });

  Screens.update('achievements', `
    <div class="container-full">
      <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content">
        <header class="text-center">
          <h1>Achievements</h1>
          <button class="box" style="margin: 1em auto;" onclick="$('achievements').classList.toggle('show-completed')">TOGGLE COMPLETED</button>
        </header>
        <achievements>
          <ul class="flex-row-2">${list}</ul>
        </achievements>
      </div>
    </div>
  `);
}

Screens.add('achievements-unlocked', (aids) => {
  const achievements = Config.get('achievements');
  let list = '';

  achievements.forEach((achievement) => {
    if (aids.indexOf(achievement.id) >= 0) {
      list += `
        <div class="blackbox" style="margin-bottom: 1em;">
          <header>
            <label>${achievement.title}</label>
            <currency class="credits inline">
              ${Icons.get('credits')}
              <amount>${achievement.credits}</amount>
            </currency>
          </header>
          <progress-bar>
            <bar style="width:100.00%"></bar>
            <span>100.00%</span>
          </progress-bar>
          <description>${achievement.description}</description>
        </div>
      `;
    }
  });

  Screens.update('achievements-unlocked', `
    <div class="container-full flex" style="background: rgba(0, 0, 0, 0.9);" onclick="Screens.hide(1); window.event.stopPropagation();">
      <div style="padding: 1em 2em; width: 100%; max-height: 100%; overflow-y: scroll;">
        <div style="max-width: 600px; margin: 0 auto;">
          <h3 class="text-center" style="margin: 0 0 1em 0">
            ${aids.length > 1 ? 'Achievements' : 'Achievement'} unlocked!
          </h3>
          <achievements>
          ${list}
          </achievements>
          <button class="box" style="width: 100%; justify-content: center;" onclick="Screens.hide(1)">
            CONTINUE
          </button>
        </div>
      </div>
    </div>
  `);

  Sound.play('win');
});
