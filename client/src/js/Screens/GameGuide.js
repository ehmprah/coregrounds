/**
 * @file
 * The game guide: getting started, pro tips, and unlock overview
 */
import Config from '../Config/Config';
import Icons from '../Screens/Elements/Icons';
import Sprites from '../Graphics/Sprites';

function gameGuideMenu(layer) {
  return `
    <div class="game-guide-menu">
      <h1 class="text-center" onclick="Screens.show('game-guide', ${layer})">Game Guide</h1>
      <ul class="content-row-3">
        <li><button class="box fit" onclick="Screens.show('game-guide-overview', ${layer}, 'tower')">Towers</button></li>
        <li><button class="box fit" onclick="Screens.show('game-guide-overview', ${layer}, 'minion')">Minions</button></li>
        <li><button class="box fit" onclick="Screens.show('game-guide-overview', ${layer}, 'ability')">Abilities</button></li>
      </ul>
    </div>
  `;
}

/**
 * Adds game guide getting started page
 */
Screens.add('game-guide', (layer) => {
  Screens.update('game-guide', `
    <div class="container-full bg-contrast" ${layer ? 'onclick="Screens.hide(1)"' : ''}>
      <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content bg-contrast" onclick="window.event.stopPropagation();">
        ${gameGuideMenu(layer)}
        <ul class="content-row-2 game-guide-landing">
          <li><button class="box fit" onclick="Screens.show('game-guide-pro-tips', ${layer})">Pro Tips</button></li>
          <li><button class="box fit" onclick="Screens.show('game-guide-detailed', ${layer})">Detailed Game Guide</button></li>
        </ul>
        <h2 class="text-center">Getting started</h2>
        <ul class="tips flex-row-2">
          <li>
            <h3>Moving Towers</h3>
            <p>Each of your towers can only exist once on the battlefield, but you can move them. Try to keep them close to the action, and retreat to heal or upgrade them.</p>
          </li>
          <li>
            <h3>Minion Stacks</h3>
            <p>Minions are automatically produced until they've reached their maximum stacks. You have to spawn minions regularly so they can keep producing!</p>
          </li>

          <li>
            <h3>Wall Stacks</h3>
            <p>You start every game with 9 free walls, but you will have to build all of these within your half of the map. Only after using the free walls you can build everywhere on the map.</p>
          </li>
          <li>
            <h3>Minion Limit</h3>
            <p>Each player can only have a total of 30 minions on the map. If you're at the minion limit, the next minion you spawn will make the foremost explode and deal its remaining health as damage to nearby enemies.</p>
          </li>

          <li>
            <h3>Upgrades</h3>
            <p>Don't forget to upgrade your towers and minions &ndash; but don't overdo it either. Knowing when to upgrade is one of the keys to mastering Coregrounds!</p>
          </li>
          <li>
            <h3>Experience Bonus</h3>
            <p>Towers and minions automatically get experience in battle and get a 2% bonus to most stats for each level, up to 20 times. If one of your units is at the maximum level, try to let others get some of the action as well.</p>
          </li>

          <li>
            <h3>Credits</h3>
            <p>You get credits for each match you play. With credits you can unlock more units, abilities and modifications.</p>
          </li>
          <li>
            <h3>Cordium</h3>
            <p>As opposed to credits, Cordium can only be purchased. They can be used to buy credit boosts or vanity items like colors, skins, emotes and backgrounds.</p>
          </li>
        </ul>
        ${layer === 0 ? `<p style="margin-top: 2em; text-align:center; color: #888;">Did you know that you can always <a href="#" onclick="Screens.show('tutorial')">replay the tutorial</a>?</p>` : ''}
      </div>
    </div>
  `);
});

/**
 * Adds game guide pro tips page
 */
Screens.add('game-guide-pro-tips', (layer) => {
  Screens.update('game-guide-pro-tips', `
    <div class="container-full bg-contrast" ${layer ? 'onclick="Screens.hide(1)"' : ''}>
      <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content bg-contrast" onclick="window.event.stopPropagation();">
        ${gameGuideMenu(layer)}
        <ul class="content-row-2 game-guide-landing">
          <li><button class="box fit" onclick="Screens.show('game-guide-pro-tips', ${layer})">Pro Tips</button></li>
          <li><button class="box fit" onclick="Screens.show('game-guide-detailed', ${layer})">Detailed Game Guide</button></li>
        </ul>
        <h2 class="text-center">Pro tips</h2>
        <ul class="tips flex-row-2">
          <li>
            <h3>Target Modes</h3>
            <p>You can set different target preference modes for most units, choosing what they should attack first.</p>
          </li>
          <li>
            <h3>Target Types</h3>
            <p>You can also toggle particular unit types altogether: you can for example assign an already leveled up tower to wall clear duty by turning off targeting for all units except walls.</p>
          </li>

          <li>
            <h3>Global Target</h3>
            <p>Click on enemy units to mark them as the global target, which will be attacked by all of your units, regardless of their target preference.</p>
          </li>
          <li>
            <h3>Modifications</h3>
            <p>Apart from units and abilities you can also unlock modifications, which provide small bonuses for your units. Modifications are pre-arranged in modification pages.</p>
          </li>

          <li>
            <h3>Upgrade limit</h3>
            <p>A unit's upgrades cannot be apart by more than two levels. This means you have to upgrade all stats to level 3 before you can max one.</p>
          </li>
          <li>
            <h3>Wall chaining</h3>
            <p>Walls touching the core or another wall get an armor bonus for each wall in the chain, up to a maximum of ten walls; this chain bonus only counts horizontally and vertically, not diagonally.</p>
          </li>

          <li>
            <h3>Early vs. Mid vs. Late Game</h3>
            <p>There are different phases to each Coregrounds match, and your loadout might be strong or weak in any of them. Knowing when to pressure your enemy is one of the keys to becoming a Coregrounds champion!</p>
          </li>
          <li>
            <h3>Range & Minion Gap</h3>
            <p>In many matches the map is full of walls. At this point it helps to have units who can shoot across the minion gap: most ranged towers for example can only do that after reaching level six for example. Try to have a long-range unit in your build or try to avoid a situation where your opponent can cross the gap while you cannot!</p>
          </li>

          <li>
            <h3>Keyboard Shortcuts</h3>
            <p>Use SPACEBAR to randomly spawn one of your minions. The TAB keys cycles between your units. 1-6 selects your units, Q,W,E selects your abilities. ESC closes overlays. CTRL / SHIFT + 1-6 / Click on a minion factory will spawn it. ALT + 1-6 / Click on any unit factory upgrades the lowest stat.</p>
          </li>
          <li>
            <h3>Touch Gestures</h3>
            <p>We're still working on these; with the mobile version of the game, every keyboard shortcut will have a corresponding touch gesture.</p>
          </li>
        </ul>
      </div>
    </div>
  `);
});

/**
 * Adds game guide unit detail page
 */
Screens.add('game-guide-detail', (id, layer) => {
  // Get the item from the config
  const item = Config.query(['factories', 'abilities'], { id });

  let strong = '';
  let weak = '';
  item.strong.forEach((counter) => {
    strong += Sprites.withBase(counter, false, layer, true, 'tiny');
  });
  item.weak.forEach((counter) => {
    weak += Sprites.withBase(counter, false, layer, true, 'tiny');
  });
  // Build HTML for the detail page
  Screens.update('game-guide-detail', `
    <div class="container-full bg-contrast" ${layer ? 'onclick="Screens.hide(1)"' : ''}>
      <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content bg-contrast" onclick="window.event.stopPropagation();">
        ${gameGuideMenu(layer)}
        <div class="game-guide-details">
          ${Sprites.withBase(item.id, false, -1, true, 'main')}
          <div class="counters">
            <p>Strong against</p>
            <div>${strong}</div>
            <p>Weak against</p>
            <div>${weak}</div>
          </div>
          <h1>${item.name}</h1>
          <p>${item.description}</p>
          <p>${item.meta}</p>
          ${parseConfigItem(item)}
        </div>
      </div>
    </div>
  `);
});

/**
 * Adds the game guide unit overview screen
 */
Screens.add('game-guide-overview', (type, layer) => {
  let html = '';
  const items = type === 'ability'
    ? Config.get('abilities')
    : Config.get('factories');

  items.forEach((item) => {
    if (type === 'ability' || item[type]) {
      html += `
        <li>
          <unlock class="blackbox" onclick="Screens.show('game-guide-detail', ${layer}, ${item.id})">
            <label>${item.name}</label>
            ${Sprites.withBase(item.id)}
          </unlock>
        </li>
      `;
    }
  });

  Screens.update('game-guide-overview', `
    <div class="container-full bg-contrast" ${layer ? 'onclick="Screens.hide(1)"' : ''}>
      <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content bg-contrast" onclick="window.event.stopPropagation();">
        ${gameGuideMenu(layer)}
        <ul class="content-row-4">${html}</ul>
      </div>
    </div>
  `);
});

/**
 * Adds the game guide overview screen
 */
Screens.add('game-guide-detailed', (layer) => {
  Screens.update('game-guide-detailed', `
    <div class="container-full bg-contrast" ${layer ? 'onclick="Screens.hide(1)"' : ''}>
      <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content bg-contrast" onclick="window.event.stopPropagation();">
        ${gameGuideMenu(layer)}
        <ul class="content-row-2 game-guide-landing">
          <li><button class="box fit" onclick="Screens.show('game-guide-pro-tips', ${layer})">Pro Tips</button></li>
          <li><button class="box fit" onclick="Screens.show('game-guide-detailed', ${layer})">Detailed Game Guide</button></li>
        </ul>
        <div class="content-text">
          <h2>In-depth game guide</h2>
          <p>Coregrounds is a cross-platform multiplayer tower defense game. It pits two players against each other on a shared battlefield spanning 15 x 7 tiles, with a core in diagonally opposing corners. Players may build walls and spawn minions which move between them. They may build and move towers on top of walls and use abilities to their advantage in order to defend their own and destroy the enemy core.</p>
          <h3>Match Phases</h3>
          <p>Coregrounds matches have two phases: the draft phase, a preparation phase during which both players pick the units, abilities and modifications they will use in the subsequent game phase.</p>
          <h3>Draft phase</h3>
          <p>Both players each ban one unit and one ability before picking five units and three abilities. One of both players will be randomly chosen to begin the draft. Players have 45 seconds to choose a unit or ability when it's their turn. If they fail to, one will be chosen for them at random. The pick and ban order is as follows:</p>
          <ol>
          <li>Player A: ban unit</li>
          <li>Player B: ban unit, ban ability</li>
          <li>Player A: ban ability, pick unit</li>
          <li>Player B: pick unit, pick unit</li>
          <li>Player A: pick unit, pick unit</li>
          <li>Player B: pick unit, pick unit</li>
          <li>Player A: pick unit, pick unit</li>
          <li>Player B: pick unit, pick ability</li>
          <li>Player A: pick ability, pick ability</li>
          <li>Player B: pick ability, pick ability</li>
          <li>Player A: pick ability</li>
          </ol>
          <p>At the end of the draft phase, players can choose their modification page. Modifications are small bonuses granted to all or specific units or abilities, which can be unlocked and configured between matches.</p>
          <h3>Game phase</h3>
          <p>The game phase will take as long as it takes either player to destroy the enemy core. If a player should disconnect, the game will continue. Players can reconnect to running games at any time. Players can also surrender matches, which will destroy their own core and make them lose immediately.</p>
          <h3>Game Modes</h3>
          <h4>Bot (unlimited credits)</h4>
          <p>Single player match against a bot.</p>
          <h4>Casual (unlimited credits)</h4>
          <p>Two player multiplayer match.</p>
          <h4>Ranked (unlimited credits)</h4>
          <p>Ranked two player multiplayer match.</p>
          <h4>Private: Casual (5 matches per day)</h4>
          <p>Two player multiplayer match; one player creates a private match and shares the access code for the other player to gain access.</p>
          <h4>Private: Random (5 matches per day)</h4>
          <p>Private match where the draft phase is automated and all units and abilities will be picked at random.</p>
          <h3>Unit types</h3>
          <h3>Core</h3>
          <p>Both player's cores are diametrally opposed in the bottom left and the top right corner of the map. Cores are the units with the most health in game, but they don't regenerate it themselves like most other units do. They can be healed by units and abilities, though. Cores gain 50% health from healing abilities than other units. Cores neither move, nor attack. Minions have attack speed reduced to 25% until they've moved out of the core.</p>
          <h3>Walls</h3>
          <p>Walls are the only unit in the game which does not need to be picked; every player always has this unit available. Walls can be placed on tiles as long as there is no other wall or unit on this tile and as long as there's a path between both Cores left. Walls obstruct the movement of minions, which can only move on tiles without walls.</p>
          <p>Walls can take damage and be destroyed, unless they are occupied by a tower. Once built, a wall can only be destroyed by damage or by certain abilities; without those, you cannot destroy your own walls. Walls touching the core or another wall get an armor bonus ("chaining") for each wall in the chain, up to a maximum of ten walls; this chain bonus only counts horizontally and vertically, not diagonally.</p>
          <p>At the beginning of the match, both players have a stack of seven walls at their disposal; they can only build farther than five (horizontal) tiles away from their core after they've placed all of these walls.</p>
          <h3>Towers</h3>
          <p>Towers can only be placed on empty walls. As opposed to walls and minions, each type of tower can only exist once on the battlefield; most towers can move between walls, though. Should a tower be destroyed, it can always be rebuilt once the tower's factory has completed the tower's 7 second death cooldown. If the tower already is on cooldown when it dies, the 7 second death cooldown will be added to the current cooldown.</p>
          <p>Apart from the death cooldown, towers have to complete cooldowns for movement, upgrades and rebuilding. On cooldown, towers can neither attack nor move, but still take damage. The movement cooldown is usually split into a windup to prepare the movement and another cooldown once it has reached the target wall. Upgrade and rebuild cooldowns depend on the amount of upgrades a tower has: Without any, building is instant and the next upgrade at 5 seconds. Both building and upgrade cooldown increase by 5 seconds for each upgrade a tower has.</p>
          <p><em class="text-gray">Caveat: All towers get a flat 30% damage reduction. Movement and upgrade cooldowns do not cancel a tower's reload progress.</em></p>
          <h3></a>Minions</h3>
          <p>Minions can not be controlled directly and move automatically along the shortest path to the enemy core. All minions will wait on collision with an ally or enemy and only move where there is room to move. Some minions ignore collision with allies and enemies, but will wait for other allied minions which ignore collision.</p>
          <p>Minions that reach the enemy core will do their remaining health as damage to the enemy Core. As opposed to towers, each minion can exist multiple times on the map. Each type has a stack limit, though: if a minion is at maximum stacks, you will have to spawn those before new ones can be produced.</p>
          <p>There also is an overall maximum of minions each player can have on the map; once this minion limit is reached, spawning a new minion will make the foremost explode for its remaining health as damage to all enemies around it. Each minion exploded this way will not trigger any on-death effects and also inflict a burn effect on <em class="text-gray">all</em> of your minion factories, which will halve their spawn speed for 2 seconds. The duration for this effect stacks, so exploding 5 minions at once will result in a 10 second minion limit burn effect.</p>
          <p><em class="text-gray">Caveat: Minions which have just been spawned will not attack until they have started moving from the core. Minions exploded via the minion limit mechanic will not execute any on-death effects.</em></p>
          <h3></a>Abilities</h3>
          <p>Abilities can be used to heal or deal damage, to buff your own, to debuff enemy units or gain various other advantages. Each player has three different abilities at their disposal for each match. Abilities can be used any time once their cooldown is ready, but they start the match on cooldown.</p>
          <h3>Factories</h3>
          <p>Walls, minions, towers and abilities are "produced" by factories. In the case of walls and abilities they don't do more than keep track of when you can build the next wall or use the next ability, but for minion and tower factories there's more:</p>
          <h3>Upgrades</h3>
          <p>Each minion or tower factory can upgrade 3 diffferent stats, each four times from level 1 to level 5. Upgrades will trigger a cooldown, during which the factory can't be used.</p>
          <p>Towers will be affected from this upgrade cooldown, while minions are not; you can't spawn new minions and no new ones are produced while you're upgrading a minion. The unit's stat is improved once the cooldown is done. Towers do not have to be rebuilt for the change to affect them; minions however are not retroactively upgraded, only new spawns after the upgrade will benefit from the upgrade.</p>
          <p>Each upgrade increases the duration for the next by 5 seconds (to a maximum of 60 seconds for the last upgrade). The difference between a units highest and lowest stat can not be more than two, so you'll have to upgrade all three stats to level 3 before you can max the first one.</p>
          <h3>Experience</h3>
          <p>Minions and towers generate experience for their factory: they generate a little bit over time and for dealing damage, taking damage, healing, and killing enemies. Some units also gain experience by helping others: the Freezer for example will get 50% of the experience another unit gets for damaging or killing a chilled unit. Attacks against the core yield triple the experience for dealing damage.</p>
          <p>How much experience factories gain depends on the unit type; units that deal damage generally don't gain experience for taking damage, and vice versa. There are also experience modifiers per unit type; some units just get less experience overall for balancing reasons. And last but not least: the amount of upgrades and experience levels a unit has also affects the amount of experience they gain: at level 0 and without upgrades they get 320% experience; for each upgrade and experience level, they get 10% less XP.</p>
          <p>Minion and tower factories can level up 20 times. Each experience level provides a 2% bonus to most stats (the exact bonus may differ from unit to unit and from stat to stat for balancing reasons).</p>
          <p><em class="text-gray">Caveat: Units spawned by other units share their parent's experience level and target preferences. Also, if multiple units kill another at the same time, the experience reward for the kill is shared.</em></p>
          <h3>Target Preferences</h3>
          <p>For most factories you can set specific target prefences and choose which enemies your units should attack. You can set different target priorities:</p>
          <ul style="margin-bottom: 1em;">
          <li>FIRST: Target the unit closest to your core</li>
          <li>HIGH: Target the unit with the highest current health</li>
          <li>LOW: Target the unit with the lowest current health</li>
          <li>RANDOM: Target a random unit in range</li>
          </ul>
          <p>This target preference will automatically prioritize unit types in the following order: Cores, Towers, Minions, Walls. If one of your units has the LOW target preference and has an enemy tower and an enemy minion in range, it will attack the tower.</p>
          <p>Apart from the target preference, you can also toggle unit types as targets altogether, and for example tell a unit to target minions and towers, but no walls.</p>
          <h2>Global Target</h2>
          <p>Apart from the automatic targeting, you can also mark enemy units as the global target, which will get attack by all of your units in range. The global target can be turned on, turned off or changed at any time.</p>
          <h2>Vision</h2>
          <p>The Coregrounds are covered by a fog of war, which units will reveal according to their range. If a unit's range covers the center of a tile, it will be fully revealed. If a unit's range covers a tile marginally only, it will be partially revealed, which means walls will be visible underneath, but towers and minions will not.</p>
          <h2>Stats</h2>
          <h3>Armor</h3>
          <p>Reduces incoming damage according to this formula:</p>
          <p>DAMAGE * 100 / (100 + ARMOR)</p>
          <h3>Armor Penetration</h3>
          <p>Is subtracted from the target unit's armor before damage is calculated.</p>
          <h3>Attack speed</h3>
          <p>The amount of times a unit can attack per second.</p>
          <h3>Damage</h3>
          <p>The amount of damage a unit can deal with its attack.</p>
          <p><em class="text-gray">Damage calculation: Damage will only be calculated for units which are not invulnerable, for example walls below a tower or units with the invulnerable status effect. If the target hast the lifelink status effect, the damage will be distributed once between all linked allies before further calculation.</em></p>
          <p><em class="text-gray">After those effects, the damage will additively include several modifiers, e.g. damage vs. entity type or damage vs. crowd controlled enemies on the attacker's side and damage reduction and armor on the target's side, before the damage will be dealt.</em></p>
          <p><em class="text-gray">Based on the amount of damage dealt, the target will now deal thorns damage, if eligible, and distribute experience to the target, the attacker and assisting units.</em></p>
          <h3>Healing</h3>
          <p>The amount of health a unit can heal with its "attack".</p>
          <p><em class="text-gray">Healing calculation: Healing will only calculated for targets which are below full health. Similar to damage, healing will be routed through the "healing received" modifier on the target side before it is applied. The healing unit will then get a percentage of the amount of health healed as experience.</em></p>
          <h3>Health</h3>
          <p>The amount of damage a unit can sustain before it is destroyed.</p>
          <h3>Movement speed</h3>
          <p>The speed at which units can move. For minions, this is the amount tiles they can move across per second.</p>
          <p><em class="text-gray">Caveat: As towers move in two steps, the movement windup and the movement cooldown, their movement speed is modifier which will affect the durations of the movement windup and cooldown.</em></p>
          <h3>Range</h3>
          <p>The distance a unit can reveal the fog of war and the maximum distance to attack a target.</p>
          <h3>Regeneration</h3>
          <p>The percantage of their maximum health a unit recovers each second.</p>
          <h3>Thorns</h3>
          <p>The percentage of damage taken a unit reflects to the attacker.</p>
          <h3>Bounces</h3>
          <p>The amount of times a unit's projectile will jump to new targets.</p>
          <h3>Chain damage</h3>
          <p>The amount of damage remaining for a projectile's next hit.</p>
          <h3>AOE Range</h3>
          <p>The range of a projectile's area of effect.</p>
          <h3>Spawn Speed</h3>
          <p>How many minions a factory can produce per second. <em class="text-gray">Caveat: Minions only.</em></p>
          <h3>Stacks</h3>
          <p>How many minions a factory can produce at a time. <em class="text-gray">Caveat: Minions only.</em></p>
          <h3>Movement Onset</h3>
          <p>The cooldown before a tower will move to a new wall. <em class="text-gray">Caveat: Towers only.</em></p>
          <h3>Movement Cooldown</h3>
          <p>The cooldown after a tower will move to a new wall. <em class="text-gray">Caveat: Towers only.</em></p>
          <h2>Status effects</h2>
          <p>Many attacks or abilities attach status effects to their targets, which are usually buffs or debuffs. Status effects affect the remainder of cooldowns, movements and attacks, based on the current progress and the (temporarily) changed stat.</p>
          <h3>Invulnerable</h3>
          <p>The unit is impervious to damage.</p>
          <h3>Rooted</h3>
          <p>The unit is unable to move.</p>
          <p><em class="text-gray">Caveat: Counts as crowd control. Towers on the move cancel their movement when rooted.</em></p>
          <h3>Stunned</h3>
          <p>The unit is neither able to move, nor attack.</p>
          <p><em class="text-gray">Caveat: Counts as crowd control. Stuns interrupt all attacks and movements.</em></p>
          <h3>Slowed</h3>
          <p>Has decreased movement speed.</p>
          <p><em class="text-gray">Caveat: Counts as crowd control.</em></p>
          <h3>Chilled</h3>
          <p>Has decreased attack and movement speed.</p>
          <p><em class="text-gray">Caveat: Counts as crowd control.</em></p>
          <h3>Lifelinked</h3>
          <p>Shares damage and healing with other lifelinked allies.</p>
          <h3>Hurried</h3>
          <p>Has increased movement speed.</p>
        </div>
      </div>
    </div>
  `);
});

function parseConfigItem(item) {
  let stats = '';

  if (item.difficulty) {
    stats += `<tr>
      <td>Difficulty</td>
      <td>${item.difficulty}</td>
    </tr>`;
  }

  // Abilities
  // TODO get damage from spells!
  // TODO get range from spells
  if (item.cooldown) {
    stats += `<tr>
      <td>Cooldown</td>
      <td>${item.cooldown} seconds</td>
    </tr>`;
  }

  // Units
  // TODO add upgrade values!
  let unitLink = false;
  if (item.minion) unitLink = item.minion;
  if (item.tower) unitLink = item.tower;

  if (unitLink) {
    // Get unit subconfig
    const unit = Config.query(['minions', 'towers'], { id: unitLink.id });

    // Add upgrades
    stats += `<tr>
      <td>Upgrades</td>
      <td>
        ${unitLink.upgrades[0].name.toUpperCase()} /
        ${unitLink.upgrades[1].name.toUpperCase()} /
        ${unitLink.upgrades[2].name.toUpperCase()}
      </td>
    </tr>`;

    if (unit.stats.vision_range) {
      stats += `<tr>
        <td>Vision Range</td>
        <td>${unit.stats.vision_range} tiles</td>
      </tr>`;
    }

    if (unit.stats.attack_range) {
      stats += `<tr>
        <td>Attack Range</td>
        <td>${unit.stats.attack_range} tiles</td>
      </tr>`;
    }

    if (unit.stats.max_health) {
      stats += `<tr>
        <td>Health</td>
        <td>${unit.stats.max_health}</td>
      </tr>`;
    }

    if (unit.stats.regeneration) {
      stats += `<tr>
        <td>Regeneration</td>
        <td>${(unit.stats.regeneration * 100).toFixed(1)}% of total health per second</td>
      </tr>`;
    }

    if (unit.stats.armor) {
      stats += `<tr>
        <td>Armor</td>
        <td>${unit.stats.armor}</td>
      </tr>`;
    }

    if (unit.stats.armor_pen) {
      stats += `<tr>
        <td>Attack Penetration</td>
        <td>${unit.stats.armor_pen}</td>
      </tr>`;
    }

    if (unit.stats.damage) {
      stats += `<tr>
        <td>Damage</td>
        <td>${unit.stats.damage}</td>
      </tr>`;
    }

    if (unit.stats.attack_speed) {
      stats += `<tr>
        <td>Attack Speed</td>
        <td>${unit.stats.attack_speed} attacks per second</td>
      </tr>`;
    }

    if (unit.stats.damage_taken) {
      stats += `<tr>
        <td>Damage Reduction</td>
        <td>${Math.floor((1 - unit.stats.damage_taken) * 100)}%</td>
      </tr>`;
    }

    // Tower movement
    if (item.tower) {
      if (unit.movement_windup) {
        stats += `<tr>
          <td>Movement Windup</td>
          <td>${unit.movement_windup} seconds</td>
        </tr>`;
      }
      if (unit.movement_cooldown) {
        stats += `<tr>
          <td>Movement Cooldown</td>
          <td>${unit.movement_cooldown} seconds</td>
        </tr>`;
      }
    }

    // Minion stacks and spawn speed
    if (item.minion) {
      // We add the movement speed here because towers use the same stat for a different purpose
      if (unit.stats.movement_speed) {
        stats += `<tr>
          <td>Movement Speed</td>
          <td>${unit.stats.movement_speed} tiles per second</td>
        </tr>`;
      }
      if (item.stats.stacks) {
        stats += `<tr>
          <td>Stacks</td>
          <td>${item.stats.stacks}</td>
        </tr>`;
      }
      if (item.stats.spawn_speed) {
        stats += `<tr>
          <td>Spawn Speed</td>
          <td>${item.stats.spawn_speed} stacks per second</td>
        </tr>`;
      }
    }
  }

  return `<table>${stats}</table>`;
}
