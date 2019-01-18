/**
 * @file Scoregrounds.js
 * Unlock page for abilities
 */
import { request } from '../API';
import { buildTabs } from '../Util/DOM';
import Icons from '../Screens/Elements/Icons';

let scores = {};

const screens = {
  'scoregrounds-bot': 'Bot',
  'scoregrounds-casual': 'Casual',
  'scoregrounds-ranked': 'Ranked',
  'scoregrounds-level': 'Level',
  'scoregrounds-achievements': 'Achievements',
};

function hourString() {
  return new Date().toJSON().slice(0, 13);
}

function decorate(type) {
  let rank;
  let html;
  let achievements;
  // Build HTML
  rank = 0;
  html = `<table><thead><tr>
      <th></th>
      <th>PLAYER</th>
      <th>BOT</th>
      <th>CASUAL</th>
      <th>RANKED</th>
      <th>RANK</th>
      <th>PLAYER LEVEL</th>
      <th>ACHIEVEMENTS</th>
    </tr></thead><tbody>`;

  scores.data[type].forEach((player) => {
    achievements = `${(parseFloat(player.achievements) * 100).toFixed(2)}%`;
    rank += 1;
    html += `<tr>
      <td>${rank}.</td>
      <td onclick="Screens.show('profile', 0, ${player.uid})">${player.name}</td>
      <td>${player.bot}</td>
      <td>${player.casual}</td>
      <td>${player.ranked}</td>
      <td>${player.rank}</td>
      <td>${player.level}</td>
      <td>
        <progress-bar>
          <bar style="width: ${achievements};"></bar>
          <span>${achievements}</span>
        </progress-bar>
      </td>
    </tr>`;
  });
  html += `</tbody></table>`;
  // Add the HTML to the page
  $('.ladder').innerHTML = html;
}

function loadData(type) {
  // Check if we have cached the scoregrounds
  const raw = window.localStorage.getItem('cgs-scoregrounds');
  if (raw) {
    scores = JSON.parse(raw);
  }

  if (scores.updated !== hourString()) {
    request('app/scoregrounds', false, (response) => {
      // Save the HTML in the local storage for the rest of the hour
      window.localStorage.setItem('cgs-scoregrounds', JSON.stringify({
        updated: hourString(),
        data: response.data,
      }));
      // Update the local copy of the data
      scores.data = response.data;
      // Decorate page with the newest data
      decorate(type);
    });
  } else {
    decorate(type);
  }
}

Screens.add('scoregrounds-bot', () => {
  Screens.update('scoregrounds-bot', `
    <div class="container-full">
      <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content">
        ${buildTabs(screens, 'scoregrounds-bot')}
        <div class="ladder">
          <throbber>LOADING</throbber>
        </div>
      </div>
    </div>
  `);
  // Load scoregrounds data
  loadData('bot');
});

Screens.add('scoregrounds-casual', () => {
  Screens.update('scoregrounds-casual', `
    <div class="container-full">
      <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content">
        ${buildTabs(screens, 'scoregrounds-casual')}
        <div class="ladder">
          <throbber>LOADING</throbber>
        </div>
      </div>
    </div>
  `);
  // Load scoregrounds data
  loadData('casual');
});

Screens.add('scoregrounds-ranked', () => {
  Screens.update('scoregrounds-ranked', `
    <div class="container-full">
      <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content">
        ${buildTabs(screens, 'scoregrounds-ranked')}
        <div class="ladder">
          <throbber>LOADING</throbber>
        </div>
      </div>
    </div>
  `);
  // Load scoregrounds data
  loadData('ranked');
});

Screens.add('scoregrounds-level', () => {
  Screens.update('scoregrounds-level', `
    <div class="container-full">
      <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content">
        ${buildTabs(screens, 'scoregrounds-level')}
        <div class="ladder">
          <throbber>LOADING</throbber>
        </div>
      </div>
    </div>
  `);
  // Load scoregrounds data
  loadData('level');
});

Screens.add('scoregrounds-achievements', () => {
  Screens.update('scoregrounds-achievements', `
    <div class="container-full">
      <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content">
        ${buildTabs(screens, 'scoregrounds-achievements')}
        <div class="ladder">
          <throbber>LOADING</throbber>
        </div>
      </div>
    </div>
  `);
  // Load scoregrounds data
  loadData('achievements');
});
