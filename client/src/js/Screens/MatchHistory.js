/**
 * @file Match.js
 * Match history screens
 *
 * TODO add share btns
 * TODO outsource the radial progress bar into own file
 */
import Config from '../Config/Config';
import { request } from '../API';
import { formatSecondDuration, formatDate } from '../Util';
import Sprites from '../Graphics/Sprites';
import Icons from '../Screens/Elements/Icons';
import { getHex } from '../Graphics/Color';
import Session from '../Session/Session';
import Event from '../Event';
import Cache from '../Util/Cache';

Event.on('game over', () => {
  // Expire the cache for the overview when we finish a match
  Cache.expire('MatchHistoryOverview');
});

Screens.add('matches', (uid) => {
  // If we didn't get a uid, we default to the currently logged in user
  if (!uid) {
    // eslint-disable-next-line
    uid = Session.getAccount().uid;
  }

  // Show throbber before we query for the current achievements
  Screens.update('matches', `
    <throbber>LOADING MATCHES</throbber>
  `);

  const data = Cache.get('MatchHistoryOverview');
  if (data) {
    buildMatchHistory(data);
  } else {
    // Send request to update the achievements
    request(`app/matches/${uid}`, false, (response) => {
      // Cache the data for an hour
      Cache.set('MatchHistoryOverview', response.data, 3600);
      if (response.data === undefined) response.data = [];
      // Build page with the newest data
      buildMatchHistory(response.data);
    });
  }
});

Screens.add('match', (mid) => {
  // Show throbber before we query for the current achievements
  Screens.update('match', `
    <throbber>LOADING MATCH</throbber>
  `);

  const data = Cache.get(`MatchHistoryMatch${mid}`);
  if (data) {
    buildMatch(data);
  } else {
    // Get match data
    request(`app/match/${mid}`, false, (response) => {
      // Cache the data for 30 days
      Cache.set(`MatchHistoryMatch${mid}`, response.data, 2592000);
      // Build page with the newest data
      buildMatch(response.data);
    });
  }
});

function buildMatchHistory(matches) {
  let list = '';
  let result;
  let picks;

  if (!matches.length) {
    return Screens.update('matches', `
      <div class="container-flex">
        <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
        <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
        No matches played yet. Go change that!
      </div>
    `);
  }

  matches.forEach((match) => {
    // Determine winner
    result = match.team === match.won
      ? '<span class="won">WON</span>'
      : '<span class="lost">LOST</span>';
    // Get icons for picks
    picks = '';
    match.picks.forEach((id) => {
      picks += Sprites.withBase(parseInt(id, 10));
    });
    // Build match HTML
    list += `
      <li class="match blackbox" onclick="Screens.show('match', 0, ${match.mid})">
        ${result}
        <span>${formatMatchType(match.match_type)}</span>
        <span>${formatSecondDuration(match.duration)}</span>
        <currency class="credits inline">
          ${Icons.get('credits')}
          <amount>${match.coins}</amount>
        </currency>
        <span>${match.xp} XP</span>
        <sprites>${picks}</sprites>
      </li>
    `;
  });

  Screens.update('matches', `
    <div class="container-full">
      <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content">
        <ul class="matches">${list}</ul>
      </div>
    </div>
  `);
}

function buildMatch(match) {
  const abbreviations = Config.get('abbreviations');
  const colors = [];
  const ended = new Date(parseInt(`${match.ended}000`, 10));
  let factoryDetails = '';
  let item;
  let playerInfo = '';

  match.players.forEach((player) => {
    colors.push(player.color);

    const uid = parseInt(player.uid, 10);

    // Build basic player info
    playerInfo += `
      <li class="blackbox flex-column relative pad">
        ${player.team === match.won ? '<div class="ribbon">WON</div>' : ''}
        <player class="flex">
          <name class="text-${player.color}"${uid ? ` onclick="Screens.show('profile', 0, ${uid})"` : ''}>
            ${player.name}
          </name>
          <currency class="credits inline">
            ${Icons.get('credits')}
            <amount>${player.coins}</amount>
          </currency>
          <xp>${player.xp} XP</xp>
        </player>
    `;

    playerInfo += '<picks class="flex">';
    // Add factories to overview and build details
    player.factories.forEach((pick) => {
      // Add factories to pick overview
      item = Config.query('factories', { id: parseInt(pick.id, 10) });
      const upgrades = item.tower ? item.tower.upgrades : item.minion.upgrades;
      playerInfo += Sprites.withBase(item.id, getHex(player.color));
      // Add factory details
      factoryDetails += `
        <li class="blackbox factory-info" style="order: ${pick.pickorder}">
          ${Sprites.withBase(item.id, getHex(player.color))}
          <upgrade>
            <label>${abbreviations[upgrades[0].name]}</label>
            <div class="clearfix rank rank-${1 + parseInt(pick.upgrade_1, 10)}">
              <div></div><div></div><div></div><div></div><div></div>
            </div>
          </upgrade>
          <upgrade>
            <label>${abbreviations[upgrades[1].name]}</label>
            <div class="clearfix rank rank-${1 + parseInt(pick.upgrade_2, 10)}">
              <div></div><div></div><div></div><div></div><div></div>
            </div>
          </upgrade>
          <upgrade>
            <label>${abbreviations[upgrades[2].name]}</label>
            <div class="clearfix rank rank-${1 + parseInt(pick.upgrade_3, 10)}">
              <div></div><div></div><div></div><div></div><div></div>
            </div>
          </upgrade>
          <svg style="margin: auto;" width="50" height="50" viewbox="0 0 100 100" version="1.1" xmlns="http://www.w3.org/2000/svg">
            <circle r="40" cx="50" cy="50" fill="transparent" stroke-dasharray="251.32" stroke-dashoffset="0" stroke-width="8" stroke="#000"></circle>
            <circle r="40" cx="50" cy="50" fill="transparent" stroke-dasharray="251.32" stroke-dashoffset="${252 - (((pick.xp % 150) / 150) * Math.PI * 80)}" stroke-width="8" stroke="${getHex(player.color)}" style="transform: rotate(-90deg); transform-origin: 50%;"></circle>
            <text x="50%" y="50%" dy=".4em" fill="${getHex(player.color)}" style="text-anchor: middle; font-size: 30px;">
              ${pick.xp > 3000 ? 20 : Math.floor(pick.xp / 150)}
            </text>
           </svg>
        </li>
      `;
    });
    // Add abilities to overview
    player.abilities.forEach((pick) => {
      // Add factories to pick overview
      item = Config.query('abilities', { id: parseInt(pick.id, 10) });
      playerInfo += Sprites.withBase(item.id, getHex(player.color), -1, false);
    });

    // Add bans to overview
    item = Config.query('factories', { id: parseInt(player.ban_1, 10) });
    if (item) {
      playerInfo += Sprites.withBase(item.id, getHex(player.color), -1, false, 'ban');
    }
    item = Config.query('abilities', { id: parseInt(player.ban_2, 10) });
    if (item) {
      playerInfo += Sprites.withBase(item.id, getHex(player.color), -1, false, 'ban');
    }
    playerInfo += '</picks></li>';
  });

  Screens.update('match', `
    <div class="container-full">
      <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content">
      <match>
        <div class="text-center">
          <h2 class="first clr-text">Match Summary</h2>
          <p>Mode: ${formatMatchType(match.match_type)} +++ Duration: ${formatSecondDuration(match.duration)} +++ Date: ${formatDate(ended)} +++ <a target="_blank" href="${Config.ENDPOINT}/match/${match.mid}">PERMALINK</a></p>
        </div>
        <ul class="flex-row-2">
          ${playerInfo}
        </ul>
        <ul class="flex-row-5">
          <li class="blackbox match-stat">
            <label>SPAWNS</label>
            <data>
              <span class="text-${colors[0]}">${match.players[0].minions_spawned}</span> /
              <span class="text-${colors[1]}">${match.players[1].minions_spawned}</span>
            </data>
          </li>
          <li class="blackbox match-stat">
            <label>WALLS BUILT</label>
            <data>
              <span class="text-${colors[0]}">${match.players[0].walls_built}</span> /
              <span class="text-${colors[1]}">${match.players[1].walls_built}</span>
            </data>
          </li>
          <li class="blackbox match-stat">
            <label>WALL KILLS</label>
            <data>
              <span class="text-${colors[0]}">${match.players[0].walls_killed}</span> /
              <span class="text-${colors[1]}">${match.players[1].walls_killed}</span>
            </data>
          </li>
          <li class="blackbox match-stat">
            <label>TOWER KILLS</label>
            <data>
              <span class="text-${colors[0]}">${match.players[0].towers_killed}</span> /
              <span class="text-${colors[1]}">${match.players[1].towers_killed}</span>
            </data>
          </li>
          <li class="blackbox match-stat">
            <label>ABILITY USES</label>
            <data>
              <span class="text-${colors[0]}">${match.players[0].ability_uses}</span> /
              <span class="text-${colors[1]}">${match.players[1].ability_uses}</span>
            </data>
          </li>
          <li class="blackbox match-stat">
            <label>UPGRADES</label>
            <data>
              <span class="text-${colors[0]}">${match.players[0].upgrade_total}</span> /
              <span class="text-${colors[1]}">${match.players[1].upgrade_total}</span>
            </data>
          </li>
          <li class="blackbox match-stat">
            <label>XP GAINED</label>
            <data>
              <span class="text-${colors[0]}">${match.players[0].xp_total}</span> /
              <span class="text-${colors[1]}">${match.players[1].xp_total}</span>
            </data>
          </li>
          <li class="blackbox match-stat">
            <label>DAMAGE</label>
            <data>
              <span class="text-${colors[0]}">${(parseInt(match.players[0].damage, 10) / 1000).toFixed(1)}k</span> /
              <span class="text-${colors[1]}">${(parseInt(match.players[1].damage, 10) / 1000).toFixed(1)}k</span>
            </data>
          </li>
          <li class="blackbox match-stat">
            <label>HEALING</label>
            <data>
              <span class="text-${colors[0]}">${(parseInt(match.players[0].healing, 10) / 1000).toFixed(1)}k</span> /
              <span class="text-${colors[1]}">${(parseInt(match.players[1].healing, 10) / 1000).toFixed(1)}k</span>
            </data>
          </li>
          <li class="blackbox match-stat">
            <label>CORE</label>
            <data>
              <span class="text-${colors[0]}">${match.players[0].core_health}%</span> /
              <span class="text-${colors[1]}">${match.players[1].core_health}%</span>
            </data>
          </li>
        </ul>

        <ul class="flex-row-2">
          ${factoryDetails}
        </ul>

      </match>
      </div>
    </div>
  `);
}

function formatMatchType(type) {
  type = parseInt(type, 10);
  if (type === 0) return 'Bot';
  else if (type === 1) return 'Casual';
  else if (type === 2) return 'Ranked';
  else if (type === 3) return 'Private';
  else if (type === 4) return 'Random';
  else if (type === 5) return 'Custom Bot';
  return false;
}
