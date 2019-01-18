/**
 * @file
 * The "ingame" tutorial
 */
import TWEEN from 'tween.js';
import { buildGrid, pointsDistance } from '../Util';
import GameServer, { getUpgradableStatNames } from '../Network/GameServer';
import { request } from '../API';
import AbilityFactory from '../Game/Models/Factories/AbilityFactory';
import Analytics from '../Analytics';
import BuildOverlay from '../Graphics/Overlays/BuildOverlay';
import Emote from '../Graphics/Elements/Emote';
import Picks from '../Graphics/Elements/Picks';
import EntityPool from '../Game/EntityPool';
import Event from '../Event';
import FogOfWar from '../Graphics/Elements/FogOfWar';
import Game from '../Game/Game';
import Messages from '../Screens/Elements/Messages';
import MinionFactory from '../Game/Models/Factories/MinionFactory';
import Particles from '../Graphics/Particles/Particles';
import Ping from '../Network/Ping';
import Player from '../Game/Models/Player';
import Session from '../Session/Session';
import Sound from '../Sound';
import TowerFactory from '../Game/Models/Factories/TowerFactory';
import UserInterface from '../Interface/UserInterface';
import Visuals from '../Graphics/Visuals/Visuals';
import WallFactory from '../Game/Models/Factories/WallFactory';

let tutorial;

Event.on('cache clear', () => {
  tutorial = {
    lock: 0,
    interval: null,
    fog: null,
    build: null,
    move: null,
    picked: {
      tower: 0,
      minion: 0,
    },
  };
});

const localNetworkHandler = {
  emit(key, data) {
    if (key === 'Surrender') {
      Event.fire('game over');
      GameServer.disconnect();
      GameServer.local(false);
      if (Session.isOnboarding()) {
        Screens.show('onboarding-meta');
      } else {
        Screens.show('home');
      }
    }

    if (key === 'PlayEmote') {
      Emote.show(0, data.id);
    }

    if (key === 'Ban') {
      // Ban the selected unit
      Picks.ban(data.id, 0, 0);
      // Show next step
      Screens.show('tutorial-3');
    }

    if (key === 'Pick') {
      if (!tutorial.picked.tower && data.id < 1100) {
        Messages.show('That is a minion. For now, pick a tower!', 'error');
        Sound.play('error');
        return;
      }
      if (tutorial.picked.tower && !tutorial.picked.minion && data.id >= 1100) {
        Messages.show('You already picked a tower. Pick a minion now!', 'error');
        Sound.play('error');
        return;
      }
      if (!tutorial.picked.tower) {
        tutorial.picked.tower = data.id;
        Picks.pick(data.id, 0, 0);
        Screens.show('tutorial-pick-minion');
        return;
      }
      if (!tutorial.picked.minion) {
        tutorial.picked.minion = data.id;
        Picks.pick(data.id, 0, 1);
        Screens.show('tutorial-draft-finished');
        return;
      }
    }

    if (key === 'ActivateFactory') {
      if (data.index === 0) {
        // As long as we don't have the first wall
        if (Game.state.entities[3] === undefined) {
          if (data.position === 425985500) {
            Sound.play('build_wall');
            const guid = 3;
            Game.state.entities[guid] = EntityPool.get('Wall');
            Game.state.entities[guid].player_id = 0;
            Game.state.entities[guid].entity_id = guid;
            Game.state.entities[guid].eid = 1;
            Game.state.entities[guid].init();
            Game.state.entities[guid].health_percentage = 1;
            Game.state.entities[guid].x = 128;
            Game.state.entities[guid].y = 768;
            // Adjust stacks
            Game.state.players[0].factories[0].stacks_current = 6;
            // Adjust fog of war and build overlay
            tutorial.build[5][0] = 0;
            tutorial.build[6][1] = 0;
            tutorial.build[6][3] = 1;
            tutorial.build[5][3] = 1;
            tutorial.build[4][2] = 1;
            BuildOverlay.replace(tutorial.build);
            tutorial.fog[6][2] = 2;
            tutorial.fog[5][2] = 2;
            tutorial.fog[6][3] = 1;
            tutorial.fog[5][3] = 1;
            tutorial.fog[4][2] = 1;
            FogOfWar.replace(tutorial.fog);
            // Fire events to trigger a redraw
            Event.fire(`network update game`, 'network update');
            // Show next screen
            Screens.show('tutorial-build-wall-2');
          } else {
            Messages.show('Please build on the highlighted tile for now.', 'error');
            Sound.play('error');
          }
          return;
        }
        // As long as we don't have the second wall
        if (Game.state.entities[4] === undefined) {
          if (data.position === 360449500) {
            Sound.play('build_wall');
            const guid = 4;
            Game.state.entities[guid] = EntityPool.get('Wall');
            Game.state.entities[guid].player_id = 0;
            Game.state.entities[guid].entity_id = guid;
            Game.state.entities[guid].eid = 1;
            Game.state.entities[guid].init();
            Game.state.entities[guid].health_percentage = 1;
            Game.state.entities[guid].x = 128;
            Game.state.entities[guid].y = 640;
            // Adjust stacks
            Game.state.players[0].factories[0].stacks_current = 5;
            // Adjust fog of war and build overlay
            tutorial.build[4][0] = 0;
            tutorial.build[5][1] = 0;
            tutorial.build[4][3] = 1;
            tutorial.build[3][0] = 1;
            tutorial.build[3][1] = 1;
            tutorial.build[3][2] = 1;
            BuildOverlay.replace(tutorial.build);
            tutorial.fog[4][0] = 2;
            tutorial.fog[4][1] = 2;
            tutorial.fog[4][2] = 2;
            tutorial.fog[4][3] = 1;
            tutorial.fog[3][2] = 1;
            tutorial.fog[3][1] = 1;
            tutorial.fog[3][0] = 1;
            FogOfWar.replace(tutorial.fog);
            // Add armor bonus
            Game.state.entities[3].chainBonus = 2;
            Game.state.entities[4].chainBonus = 2;
            // Fire events to trigger a redraw
            Event.fire(`network update game`, 'network update');
            // Show next screen
            Screens.show('tutorial-build-wall-3');
          } else {
            Messages.show('Please build on the highlighted tile for now.', 'error');
            Sound.play('error');
          }
          return;
        }

        // As long as we don't have the second wall
        if (Game.state.entities[5] === undefined) {
          if (data.position === 294913500) {
            Sound.play('build_wall');
            const guid = 5;
            Game.state.entities[guid] = EntityPool.get('Wall');
            Game.state.entities[guid].player_id = 0;
            Game.state.entities[guid].entity_id = guid;
            Game.state.entities[guid].eid = 1;
            Game.state.entities[guid].init();
            Game.state.entities[guid].health_percentage = 1;
            Game.state.entities[guid].x = 128;
            Game.state.entities[guid].y = 512;
            // Adjust stacks
            Game.state.players[0].factories[0].stacks_current = 4;
            // Adjust fog of war and build overlay
            tutorial.build[3][0] = 0;
            tutorial.build[4][1] = 0;
            tutorial.build[3][3] = 1;
            tutorial.build[2][0] = 1;
            tutorial.build[2][1] = 1;
            tutorial.build[2][2] = 1;
            BuildOverlay.replace(tutorial.build);
            tutorial.fog[3][0] = 2;
            tutorial.fog[3][1] = 2;
            tutorial.fog[3][2] = 2;
            tutorial.fog[3][3] = 1;
            tutorial.fog[2][2] = 1;
            tutorial.fog[2][1] = 1;
            tutorial.fog[2][0] = 1;
            FogOfWar.replace(tutorial.fog);
            // Add armor bonus
            Game.state.entities[3].chainBonus = 3;
            Game.state.entities[4].chainBonus = 3;
            Game.state.entities[5].chainBonus = 3;
            // Fire events to trigger a redraw
            Event.fire(`network update game`, 'network update');
            // Show next screen
            Screens.show('tutorial-build-wall-4');
          } else {
            Messages.show('Please build on the highlighted tile for now.', 'error');
            Sound.play('error');
          }
          return;
        }
        // As long as we don't have the second wall
        if (Game.state.entities[6] === undefined) {
          if (data.position === 229377500) {
            Sound.play('build_wall');
            const guid = 6;
            Game.state.entities[guid] = EntityPool.get('Wall');
            Game.state.entities[guid].player_id = 0;
            Game.state.entities[guid].entity_id = guid;
            Game.state.entities[guid].eid = 1;
            Game.state.entities[guid].init();
            Game.state.entities[guid].health_percentage = 1;
            Game.state.entities[guid].x = 128;
            Game.state.entities[guid].y = 384;
            // Adjust stacks
            Game.state.players[0].factories[0].stacks_current = 3;
            // Adjust fog of war and build overlay
            tutorial.build[2][0] = 0;
            tutorial.build[3][1] = 0;
            tutorial.build[2][3] = 1;
            tutorial.build[1][0] = 1;
            tutorial.build[1][1] = 1;
            tutorial.build[1][2] = 1;
            BuildOverlay.replace(tutorial.build);
            tutorial.fog[2][0] = 2;
            tutorial.fog[2][1] = 2;
            tutorial.fog[2][2] = 2;
            tutorial.fog[2][3] = 1;
            tutorial.fog[1][2] = 1;
            tutorial.fog[1][1] = 1;
            tutorial.fog[1][0] = 1;
            FogOfWar.replace(tutorial.fog);
            // Add armor bonus
            Game.state.entities[3].chainBonus = 4;
            Game.state.entities[4].chainBonus = 4;
            Game.state.entities[5].chainBonus = 4;
            Game.state.entities[6].chainBonus = 4;
            // Fire events to trigger a redraw
            Event.fire(`network update game`, 'network update');
            // Show next screen
            Screens.show('tutorial-build-wall-5');
          } else {
            Messages.show('Please build on the highlighted tile for now.', 'error');
            Sound.play('error');
          }
          return;
        }
        // As long as we don't have the second wall
        if (Game.state.entities[7] === undefined) {
          if (data.position === 163841500) {
            Sound.play('build_wall');
            const guid = 7;
            Game.state.entities[guid] = EntityPool.get('Wall');
            Game.state.entities[guid].player_id = 0;
            Game.state.entities[guid].entity_id = guid;
            Game.state.entities[guid].eid = 1;
            Game.state.entities[guid].init();
            Game.state.entities[guid].health_percentage = 1;
            Game.state.entities[guid].x = 128;
            Game.state.entities[guid].y = 256;
            // Adjust stacks
            Game.state.players[0].factories[0].stacks_current = 2;
            // Adjust fog of war and build overlay
            tutorial.build[1][0] = 0;
            tutorial.build[2][1] = 0;
            tutorial.build[1][3] = 1;
            tutorial.build[0][0] = 1;
            tutorial.build[0][1] = 1;
            tutorial.build[0][2] = 1;
            BuildOverlay.replace(tutorial.build);
            tutorial.fog[1][0] = 2;
            tutorial.fog[1][1] = 2;
            tutorial.fog[1][2] = 2;
            tutorial.fog[1][3] = 1;
            tutorial.fog[0][2] = 1;
            tutorial.fog[0][1] = 1;
            tutorial.fog[0][0] = 1;
            FogOfWar.replace(tutorial.fog);
            // Add armor bonus
            Game.state.entities[3].chainBonus = 5;
            Game.state.entities[4].chainBonus = 5;
            Game.state.entities[5].chainBonus = 5;
            Game.state.entities[6].chainBonus = 5;
            Game.state.entities[7].chainBonus = 5;
            // Fire events to trigger a redraw
            Event.fire(`network update game`, 'network update');
            // Show next screen
            Screens.show('tutorial-build-wall-6');
          } else {
            Messages.show('Please build on the highlighted tile for now.', 'error');
            Sound.play('error');
          }
          return;
        }
        // As long as we don't have the second wall
        if (Game.state.entities[8] === undefined) {
          if (data.position === 98305500) {
            Sound.play('build_wall');
            const guid = 8;
            Game.state.entities[guid] = EntityPool.get('Wall');
            Game.state.entities[guid].player_id = 0;
            Game.state.entities[guid].entity_id = guid;
            Game.state.entities[guid].eid = 1;
            Game.state.entities[guid].init();
            Game.state.entities[guid].health_percentage = 1;
            Game.state.entities[guid].x = 128;
            Game.state.entities[guid].y = 128;
            // Adjust stacks
            Game.state.players[0].factories[0].stacks_current = 1;
            // Adjust fog of war and build overlay
            tutorial.build[0][0] = 0;
            tutorial.build[0][1] = 0;
            tutorial.build[0][2] = 0;
            tutorial.build[0][3] = 1;
            tutorial.build[1][1] = 0;
            BuildOverlay.replace(tutorial.build);
            tutorial.fog[0][3] = 1;
            tutorial.fog[0][2] = 2;
            tutorial.fog[0][1] = 2;
            tutorial.fog[0][0] = 2;
            FogOfWar.replace(tutorial.fog);
            // Add armor bonus
            Game.state.entities[3].chainBonus = 6;
            Game.state.entities[4].chainBonus = 6;
            Game.state.entities[5].chainBonus = 6;
            Game.state.entities[6].chainBonus = 6;
            Game.state.entities[7].chainBonus = 6;
            Game.state.entities[8].chainBonus = 6;
            // Fire events to trigger a redraw
            Event.fire(`network update game`, 'network update');
            // Show next screen
            Screens.show('tutorial-walls-finished');
            // We reset the interface here to not encourage the user to build more walls
            UserInterface.reset();
            // Lock the wall factory for now
            tutorial.locked = 1;
          } else {
            Messages.show('Please build on the highlighted tile for now.', 'error');
            Sound.play('error');
          }
          return;
        }
        Messages.show('Please focus on the next tutorial steps for now!', 'error');
        Sound.play('error');
      }
      if (data.index === 1) {
        if (Game.state.players[0].factories[1].cooldown_seconds !== 0) {
          Sound.play('error');
          return;
        }
        // Build the tower
        if (Game.state.entities[9] === undefined) {
          Sound.play('build_tower');
          const x = Math.round((data.position & 32767) * 0.128) - 64;
          const y = Math.round((data.position >> 16) * 0.128) - 64;
          const guid = 9;
          Game.state.entities[guid] = EntityPool.get('Tower');
          Game.state.entities[guid].player_id = 0;
          Game.state.entities[guid].entity_id = guid;
          Game.state.entities[guid].eid = tutorial.picked.tower;
          Game.state.entities[guid].init();
          Game.state.entities[guid].health_percentage = 1;
          Game.state.entities[guid].attackRadius = 1.5;
          Game.state.entities[guid].x = x;
          Game.state.entities[guid].y = y;
          Game.state.players[0].factories[1].entity_id = guid;
          Game.state.players[0].factories[1].entityBox.x = x;
          Game.state.players[0].factories[1].entityBox.y = y;
          Game.state.players[0].factories[1].entityBox.listening = true;
          // Build cooldown
          Game.state.entities[9].cooldown_seconds = 2;
          Game.state.players[0].factories[1].cooldown_seconds = 2;
          new TWEEN.Tween(Game.state.players[0].factories[1])
            .to({ cooldown_percentage: 1, cooldown_seconds: 0 }, 2000)
            .onUpdate(function updateTowerEntity() {
              Game.state.entities[9].cooldown_percentage = this.cooldown_percentage;
              Game.state.entities[9].cooldown_seconds = this.cooldown_seconds;
            })
            .onComplete(() => {
              Game.state.entities[9].cooldown_percentage = 0;
              Game.state.players[0].factories[1].cooldown_percentage = 0;
            })
            .start();
          // Show next screen
          Screens.show('tutorial-move-tower');
        } else {
          const x = Math.round((data.position & 32767) * 0.128) - 64;
          const y = Math.round((data.position >> 16) * 0.128) - 64;
          // Set cooldown
          Game.state.entities[9].cooldown_seconds = 2;
          Game.state.players[0].factories[1].cooldown_seconds = 2;
          // TWEEN the rest
          const second = new TWEEN.Tween(Game.state.entities[9])
            .to({ cooldown_percentage: 1, cooldown_seconds: 0 }, 2000)
            .onUpdate(function updateTowerEntity() {
              Game.state.players[0].factories[1].cooldown_percentage = this.cooldown_percentage;
              Game.state.players[0].factories[1].cooldown_seconds = this.cooldown_seconds;
            })
            .onComplete(() => {
              Game.state.entities[9].cooldown_percentage = 0;
              Game.state.players[0].factories[1].cooldown_percentage = 0;
              Screens.show('tutorial-upgrade-tower');
            });
          new TWEEN.Tween(Game.state.entities[9])
            .to({ cooldown_percentage: 1, cooldown_seconds: 0 }, 2000)
            .onUpdate(function updateTowerEntity() {
              Game.state.players[0].factories[1].cooldown_percentage = this.cooldown_percentage;
              Game.state.players[0].factories[1].cooldown_seconds = this.cooldown_seconds;
            })
            .onComplete(() => {
              Game.state.entities[9].x = x;
              Game.state.entities[9].y = y;
              Game.state.entities[9].cooldown_percentage = 0;
              Game.state.players[0].factories[1].cooldown_percentage = 0;
              Game.state.entities[9].cooldown_seconds = 2;
              Game.state.players[0].factories[1].cooldown_seconds = 2;
              Game.state.players[0].factories[1].entityBox.x = x;
              Game.state.players[0].factories[1].entityBox.y = y;
            })
            .chain(second)
            .start();
        }
      }

      if (data.index === 2) {
        // Reset and lock the factory
        UserInterface.reset();
        tutorial.locked = 3;
        // Adjust stacks
        Game.state.players[0].factories[2].stacks_current = 0;
        Sound.play('spawn_minion');
        const guid = 10;
        Game.state.entities[guid] = EntityPool.get('Minion');
        Game.state.entities[guid].player_id = 0;
        Game.state.entities[guid].entity_id = guid;
        Game.state.entities[guid].eid = tutorial.picked.minion;
        Game.state.entities[guid].init();
        Game.state.entities[guid].health_percentage = 1;
        Game.state.entities[guid].attackRadius = 1.5;
        Game.state.entities[guid].x = 0;
        Game.state.entities[guid].y = 768;
        Game.state.entities[guid].angle = 1.5 * Math.PI;
        // Build movement
        const fog = JSON.parse(JSON.stringify(tutorial.fog));
        const second = new TWEEN.Tween(Game.state.entities[10])
          .to({ x: 1760 }, 9000)
          .onUpdate(function updateMinion() {
            // Check vision
            const updated = JSON.parse(JSON.stringify(fog));
            let tile = Math.floor(this.x / 128);
            if ((this.x / 128) % 1 >= 0.5) {
              tile += 1;
            }
            if (tile > 2) {
              updated[0][tile] = 2;
              updated[0][tile - 1] = 1;
              updated[0][tile + 1] = 1;
              updated[1][tile] = 1;
              FogOfWar.replace(updated);
            }
          })
          .onComplete(() => {
            Game.state.entities[2].health_percentage = 0.25;
            Sound.play('explosion');
            Particles.spriteExplosion(JSON.parse(JSON.stringify(Game.state.entities[10])));
            delete Game.state.entities[10];
            FogOfWar.replace(JSON.parse(JSON.stringify(fog)));
          });
        new TWEEN.Tween(Game.state.entities[10])
          .to({ y: 0 }, 3600)
          .onComplete(function updateMinion() {
            this.angle = 0;
          })
          .chain(second)
          .start();
        Screens.show('tutorial-minion-moving');
      }
    }

    if (key === 'UpgradeFactory') {
      if (data.index === 1) {
        if (Game.state.players[0].factories[1].cooldown_seconds !== 0) {
          Sound.play('error');
          return;
        }
        Game.state.entities[9].cooldown_seconds = 5;
        Game.state.players[0].factories[1].cooldown_seconds = 5;
        new TWEEN.Tween(Game.state.players[0].factories[1])
          .to({ cooldown_percentage: 1, cooldown_seconds: 0 }, 5000)
          .onUpdate(function updateTowerEntity() {
            Game.state.entities[9].cooldown_percentage = this.cooldown_percentage;
            Game.state.entities[9].cooldown_seconds = this.cooldown_seconds;
          })
          .onComplete(() => {
            Game.state.entities[9].cooldown_percentage = 0;
            Game.state.players[0].factories[1].cooldown_percentage = 0;
          })
          .start();
        Screens.show('tutorial-upgrade-issued');
        // And we turn off the listener for walls
        Game.state.players[0].factories[1].box.listening = false;
        Game.state.players[0].factories[1].entityBox.listening = false;
        // Lock the wall factory for now
        tutorial.locked = 2;
      } else {
        Messages.show('Please spawn the minion.', 'error');
        Sound.play('error');
      }
    }

    if (key === 'ActivateAbility') {
      const x = Math.round((data.position & 32767) * 0.128);
      const y = Math.round((data.position >> 16) * 0.128);
      Visuals.show(1401, 0, x, y);
      if (pointsDistance(x, y, 1856, 64) <= 105) {
        setTimeout(() => {
          Screens.show('tutorial-finished');
        }, 1500);
      } else {
        Messages.show('Close, but no cigar. Aim for the core!', 'error');
        Sound.play('error');
      }
    }

    if (key === 'ping') Ping.update();
  },

  close() {
    if (tutorial.interval) {
      clearInterval(tutorial.interval);
    }
  },
};

/**
 * The tutorial splash screen
 */
Screens.add('tutorial', () => {
  Screens.update('tutorial', `
    <div class="text-center">
      <h1 class="first">TUTORIAL</h1>
      <p>This tutorial will teach you the basics in about five minutes.</p>
      <div class="flex" style="justify-content: space-around;">
        <a href="#" id="skip-tutorial">Skip tutorial</a>
        <button class="box" onclick="Screens.show('tutorial-1')">Start tutorial</button>
      </div>
    </div>
  `);

  $('#skip-tutorial').addEventListener('click', () => {
    // Send request to webserver
    request('app/complete-tutorial', false, () => {
      Analytics.event('Tutorial', 'skip');
      // We don't need to update the data via the server
      const account = Session.getAccount();
      account.tutorial = 1;
      Session.setAccount(account);
      if (Session.isOnboarding()) {
        Screens.show('onboarding-meta');
      } else {
        Screens.show('home');
      }
    });
  });
});

/**
 * Tutorial step 1
 */
Screens.add('tutorial-1', () => {
  // Create "connection" to our tutorial network handler
  GameServer.local(localNetworkHandler);
  // Create players
  Game.state.players.push(new Player());
  Game.state.players.push(new Player());
  // Set colors
  Game.state.players[0].color = Session.getAccount().color;
  Game.state.players[1].color = 2400;
  Game.playerIndex = 0;
  Event.fire('match colors updated', 'game started');
  // Change game state for this particular tutorial step
  Game.status = 1;
  Game.state.draft = 1;
  Game.state.time = 45;
  // Fire events
  Event.fire('before draft phase update');
  Event.fire('draft phase', 'status changed');
  Event.fire(`network update draft`, 'network update');
  // Update tutorial step on top
  Screens.update('tutorial-1', `
    <div class="tutorial-wrapper">
      <div class="tutorial-box clickcapture" style="width: 60%;">
        <h3>DRAFT PHASE</h3>
        <p>Every Coregrounds game starts with the draft phase, where you take turns to choose the units you want to take into battle. Each unit can only be picked once.</p>
        <button class="box" onclick="Screens.show('tutorial-2')">GOT IT</button>
      </div>
    </div>
  `).addClass('clickthrough');
});

/**
 * Tutorial step 2
 */
Screens.add('tutorial-2', () => {
  // Now it's the player's turn
  Game.activePlayerIndex = 0;
  // Add three factories to choose from
  Game.state.players[0].pickable.push(1001, 1002, 1006, 1100, 1101, 1113);
  // Fire events
  Event.fire('updated available picks');
  Event.fire(`network update draft`, 'network update');
  // Update screen
  Screens.update('tutorial-2', `
    <div class="tutorial-wrapper">
      <div class="tutorial-box triangle-bottom clickcapture" style="width: 50%; top: 5%;">
        <h3>BAN A UNIT</h3>
        <p>The unit you ban will not be pickable anymore, neither for you, nor your opponent.</p>
      </div>
    </div>
  `).addClass('clickthrough');
});

/**
 * Tutorial step 3
 */
Screens.add('tutorial-3', () => {
  Game.activePlayerIndex = 1;
  // Fire events
  Event.fire('updated available picks');
  Event.fire(`network update draft`, 'network update');
  // Update screen
  Screens.update('tutorial-3', `
    <div class="tutorial-wrapper">
      <div class="tutorial-box clickcapture" style="width: 60%;">
        <h3>GOOD JOB</h3>
        <p>Now it's your opponent's turn to ban. Because you take turns, you can react and adapt your strategy to your opponent's picks!</p>
        <button class="box" onclick="Screens.show('tutorial-pick-tower')">UNDERSTOOD</button>
      </div>
    </div>
  `).addClass('clickthrough');
});

/**
 * Tutorial step 3
 */
Screens.add('tutorial-pick-tower', () => {
  // Ban something for the opponent
  Picks.pick(1112, 1, 0);
  // The player's turn again
  Game.activePlayerIndex = 0;
  // We're picking now
  Game.state.draft = 3;
  // Fire events
  Event.fire('updated available picks');
  Event.fire(`network update draft`, 'network update');
  // Update screen
  Screens.update('tutorial-pick-tower', `
    <div class="tutorial-wrapper">
      <div class="tutorial-box triangle-bottom clickcapture" style="width: 60%; right: 20%; top: 5%;">
        <h3>PICK A TOWER</h3>
        <p>Now it's your turn again. Choose and pick a tower.</p>
      </div>
    </div>
  `).addClass('clickthrough');
});

/**
 * Tutorial step 3
 */
Screens.add('tutorial-pick-minion', () => {
  // Fire events
  Event.fire('updated available picks');
  Event.fire(`network update draft`, 'network update');
  // Update screen
  Screens.update('tutorial-pick-minion', `
    <div class="tutorial-wrapper">
      <div class="tutorial-box triangle-bottom clickcapture" style="width: 60%; right: 20%; top: 5%;">
        <h3>PICK A MINION</h3>
        <p>Now choose a minion.</p>
      </div>
    </div>
  `).addClass('clickthrough');
});

/**
 * Tutorial step 4
 */
Screens.add('tutorial-draft-finished', () => {
  // // Play sound
  // Sound.play('win');
  // Now it's the other side's turn
  Game.activePlayerIndex = 1;
  // Fire events
  Event.fire('updated available picks');
  Event.fire(`network update draft`, 'network update');
  // Update screen
  Screens.update('tutorial-draft-finished', `
    <div class="tutorial-wrapper">
      <div class="tutorial-box clickcapture" style="width: 60%;">
        <h3>Great! You're a natural!</h3>
        <p>We'll skip ahead to the game phase for now. You will learn about units and draft strategies as you play.</p>
        <button class="box" onclick="Screens.show('tutorial-5')">OK, let's go</button>
      </div>
    </div>
  `).addClass('clickthrough');
});

/**
 * Tutorial step 5
 */
Screens.add('tutorial-5', () => {
  // Change game state for this particular tutorial step
  Game.status = 2;
  Game.state.time = 0;
  Event.fire('game phase', 'status changed');

  // Start an interval for the "network" update
  tutorial.interval = setInterval(() => {
    Event.fire(`network update game`, 'network update');
    // Lock factories after they've served their tutorial purpose
    if (tutorial.locked > 0) {
      Game.state.players[0].factories[0].box.listening = false;
    }
    if (tutorial.locked > 1) {
      Game.state.players[0].factories[1].box.listening = false;
      Game.state.players[0].factories[1].entityBox.listening = false;
    }
    if (tutorial.locked > 2) {
      Game.state.players[0].factories[2].box.listening = false;
    }
  }, 50);

  // Add cores
  [1, 2].forEach((guid) => {
    Game.state.entities[guid] = EntityPool.get('Core');
    Game.state.entities[guid].player_id = guid - 1;
    Game.state.entities[guid].entity_id = guid;
    Game.state.entities[guid].eid = 1;
    Game.state.entities[guid].init();
    Game.state.entities[guid].health_percentage = 1;
    Game.state.entities[guid].x = guid === 1 ? 0 : 1792;
    Game.state.entities[guid].y = guid === 1 ? 768 : 0;
  });

  // Add building map
  tutorial.build = buildGrid();
  tutorial.build[6][1] = 1;
  tutorial.build[6][2] = 1;
  tutorial.build[5][0] = 1;
  tutorial.build[5][1] = 1;
  tutorial.build[5][2] = 1;
  tutorial.build[4][0] = 1;
  tutorial.build[4][1] = 1;
  BuildOverlay.replace(tutorial.build);

  // Add fog of war map
  tutorial.fog = buildGrid();
  tutorial.fog[6][0] = 2;
  tutorial.fog[6][1] = 2;
  tutorial.fog[5][0] = 2;
  tutorial.fog[5][1] = 2;
  tutorial.fog[6][2] = 1;
  tutorial.fog[5][2] = 1;
  tutorial.fog[4][1] = 1;
  tutorial.fog[4][0] = 1;
  tutorial.fog[0][14] = 1;
  FogOfWar.replace(tutorial.fog);

  Event.fire('slow network update');
  Event.fire(`network update game`, 'network update');
  // Update screen
  Screens.update('tutorial-5', `
    <div class="tutorial-wrapper">
      <div class="tutorial-box clickcapture" style="width: 60%; top: 20%;">
        <h3>WELCOME TO THE COREGROUNDS</h3>
        <p>After the draft phase the game begins: and this is the battlefield, the so-called "Coregrounds".</p>
        <button class="box" onclick="Screens.show('tutorial-6')">UNDERSTOOD</button>
      </div>
    </div>
  `);
});

Screens.add('tutorial-6', () => {
  // Update screen
  Screens.update('tutorial-6', `
    <div class="tutorial-wrapper">
      <div class="tutorial-box triangle-left bottom clickcapture" style="width: 50%; left: 10%; bottom: 15%;">
        <h3>THIS IS YOUR CORE</h3>
        <p>Defend this at all costs. Destroy the enemy core to win. But first let's build some walls!</p>
        <button class="box" onclick="Screens.show('tutorial-build-wall')">ALRIGHT</button>
      </div>
    </div>
  `).addClass('clickthrough');
});

Screens.add('tutorial-build-wall', () => {
  // Add wall factory in this step
  const { factories } = Game.state.players[Game.playerIndex];
  factories.push(new WallFactory(0, Game.playerIndex));
  factories[0].stacks_current = 7;
  factories[0].stacks_max = 1;

  // Fire events
  Event.fire('slow network update', `network update game`, 'network update');
  // Update screen
  Screens.update('tutorial-build-wall', `
    <div class="tutorial-wrapper">
      <div class="tutorial-box clickcapture triangle-left bottom" style="width: 60%; left: 20%; bottom: 1%;">
        <h3>BUILD WALLS NOW</h3>
        <p>Select your wall or start dragging to see where you can build walls. For now, build one to the right of your core.</p>
      </div>
      <div class="tutorial-highlight" style="left: 4em; top: 24em;"></div>
    </div>
  `).addClass('clickthrough');
});

Screens.add('tutorial-build-wall-2', () => {
  // Update screen
  Screens.update('tutorial-build-wall-2', `
    <div class="tutorial-wrapper">
      <div class="tutorial-box clickcapture bottom" style="width: 50%;">
        <h3>GOOD JOB!</h3>
        <p>Now continue the wall line to the top.</p>
      </div>
      <div class="tutorial-highlight" style="left: 4em; top: 20em;"></div>
    </div>
  `).addClass('clickthrough');
});

Screens.add('tutorial-build-wall-3', () => {
  // Update screen
  Screens.update('tutorial-build-wall-3', `
    <div class="tutorial-wrapper">
      <div class="tutorial-box clickcapture bottom" style="width: 50%;">
        <h3>GOOD JOB!</h3>
        <p>Now continue the wall line to the top.</p>
      </div>
      <div class="tutorial-highlight" style="left: 4em; top: 16em;"></div>
    </div>
  `).addClass('clickthrough');
});

Screens.add('tutorial-build-wall-4', () => {
  // Update screen
  Screens.update('tutorial-build-wall-4', `
    <div class="tutorial-wrapper">
      <div class="tutorial-box clickcapture bottom" style="width: 50%;">
        <h3>GOOD JOB!</h3>
        <p>Now continue the wall line to the top.</p>
      </div>
      <div class="tutorial-highlight" style="left: 4em; top: 12em;"></div>
    </div>
  `).addClass('clickthrough');
});

Screens.add('tutorial-build-wall-5', () => {
  // Update screen
  Screens.update('tutorial-build-wall-5', `
    <div class="tutorial-wrapper">
      <div class="tutorial-box clickcapture bottom" style="width: 50%;">
        <h3>GOOD JOB!</h3>
        <p>Now continue the wall line to the top.</p>
      </div>
      <div class="tutorial-highlight" style="left: 4em; top: 8em;"></div>
    </div>
  `).addClass('clickthrough');
});

Screens.add('tutorial-build-wall-6', () => {
  // Update screen
  Screens.update('tutorial-build-wall-6', `
    <div class="tutorial-wrapper">
      <div class="tutorial-box clickcapture bottom" style="width: 50%;">
        <h3>GOOD JOB!</h3>
        <p>Now continue the wall line to the top.</p>
      </div>
      <div class="tutorial-highlight" style="left: 4em; top: 4em;"></div>
    </div>
  `).addClass('clickthrough');
});

Screens.add('tutorial-walls-finished', () => {
  // Update screen
  Screens.update('tutorial-walls-finished', `
    <div class="tutorial-wrapper">
      <div class="tutorial-box clickcapture bottom" style="width: 50%;">
        <h3>GREAT JOB!</h3>
        <p>Enemy minions will have to go around those walls to reach your core. Also, walls are the foundation for your tower, which you'll build next.</p>
        <button class="box" onclick="Screens.show('tutorial-build-tower')">OK, let's go</button>
      </div>
    </div>
  `).addClass('clickthrough');
});

Screens.add('tutorial-build-tower', () => {
  const { factories } = Game.state.players[Game.playerIndex];
  // Now add the tower factory
  factories.push(new TowerFactory(tutorial.picked.tower, 1, 0));
  factories[1].upgradableStatNames = getUpgradableStatNames(tutorial.picked.tower);
  factories[1].localIndex = 1;

  // Fire events
  Event.fire('slow network update', `network update game`, 'network update');
  // Update screen
  Screens.update('tutorial-build-tower', `
    <div class="tutorial-wrapper">
      <div class="tutorial-box clickcapture triangle-left bottom" style="width: 60%; left: 20%; bottom: 1%;">
        <h3>BUILD YOUR TOWER</h3>
        <p>Select your tower or start dragging and build it on one of your walls.</p>
      </div>
    </div>
  `).addClass('clickthrough');
});

Screens.add('tutorial-move-tower', () => {
  // Update screen
  Screens.update('tutorial-move-tower', `
    <div class="tutorial-wrapper">
      <div class="tutorial-box clickcapture bottom" style="width: 50%;">
        <h3>NICE WORK!</h3>
        <p>Each tower can only exist once on the battlefield. You can move it, though. Select it or start dragging and move it to another wall.</p>
      </div>
    </div>
  `).addClass('clickthrough');
});

Screens.add('tutorial-upgrade-tower', () => {
  Sound.play('notification');
  // Update screen
  Screens.update('tutorial-upgrade-tower', `
    <div class="tutorial-wrapper">
      <div class="tutorial-box clickcapture bottom" style="width: 50%;">
        <h3>UPGRADE YOUR TOWER</h3>
        <p>Your tower and minion factories each have three stats which can be upgraded. Select your tower and upgrade one stat.</p>
      </div>
    </div>
  `).addClass('clickthrough');
});

Screens.add('tutorial-upgrade-issued', () => {
  // Update screen
  Screens.update('tutorial-upgrade-issued', `
    <div class="tutorial-wrapper">
      <div class="tutorial-box clickcapture bottom" style="width: 50%;">
        <h3>GOOD JOB!</h3>
        <p>Use upgrades to gain an advantage, but take care: each upgrade increases the cooldown duration.</p>
        <button class="box" onclick="Screens.show('tutorial-xp')">GOT IT</button>
      </div>
    </div>
  `).addClass('clickthrough');
});

Screens.add('tutorial-xp', () => {
  // Update screen
  Screens.update('tutorial-xp', `
    <div class="tutorial-wrapper">
      <div class="tutorial-box clickcapture bottom" style="width: 50%;">
        <h3>EXPERIENCE</h3>
        <p>Your units also get experience: over time, by killing enemy units or by taking damage.
        The current level and the progress to the next is displayed in the little colored box to
        the bottom for each unit. Each level grants a 3% bonus on most stats.</p>
        <button class="box" onclick="Screens.show('tutorial-spawn-minion')">GOT IT</button>
      </div>
    </div>
  `).addClass('clickthrough');
});

Screens.add('tutorial-spawn-minion', () => {
  const { factories } = Game.state.players[Game.playerIndex];
  // Now add the tower factory
  factories.push(new MinionFactory(tutorial.picked.minion, 2, 0));
  factories[2].upgradableStatNames = getUpgradableStatNames(tutorial.picked.minion);
  factories[2].localIndex = 2;
  factories[2].stacks_current = 1;
  factories[2].stacks_max = 1;

  // Update screen
  Screens.update('tutorial-spawn-minion', `
    <div class="tutorial-wrapper">
      <div class="tutorial-box clickcapture triangle-left bottom" style="width: 60%; left: 25%; bottom: 1%;">
        <h3>SPAWN A MINION</h3>
        <p>Select your minion or start dragging and release or click anywhere on the map to spawn it.</p>
      </div>
    </div>
  `).addClass('clickthrough');
});

Screens.add('tutorial-minion-moving', () => {
  // Update screen
  Screens.update('tutorial-minion-moving', `
    <div class="tutorial-wrapper">
      <div class="tutorial-box clickcapture bottom" style="width: 50%;">
        <h3>GREAT JOB!</h3>
        <p>Minions automatically move towards the enemy core, where they will do their remaining health as damage.</p>
        <button class="box" onclick="Screens.show('tutorial-core-damage')">ALRIGHT</button>
      </div>
    </div>
  `).addClass('clickthrough');
});

Screens.add('tutorial-core-damage', () => {
  // Update screen
  Screens.update('tutorial-core-damage', `
    <div class="tutorial-wrapper">
      <div class="tutorial-box clickcapture bottom" style="width: 50%;">
        <h3>SO CLOSE</h3>
        <p>Your minion has almost destroyed the enemy core. Use an ability now to finish it off!</p>
        <button class="box" onclick="Screens.show('tutorial-use-ability')">OK, let's go</button>
      </div>
    </div>
  `).addClass('clickthrough');
});

Screens.add('tutorial-use-ability', () => {
  const { factories } = Game.state.players[Game.playerIndex];
  // Now add the tower factory
  factories.push({ draw: () => {}, box: { listening: true } });
  factories.push({ draw: () => {}, box: { listening: true } });
  factories.push({ draw: () => {}, box: { listening: true } });
  factories.push(new AbilityFactory(1401, 6, 0));
  factories[6].box.x -= 384;

  // Update screen
  Screens.update('tutorial-use-ability', `
    <div class="tutorial-wrapper">
      <div class="tutorial-box clickcapture triangle-left bottom" style="width: 60%; left: 33%; bottom: 1%;">
        <h3>USE ABILITY</h3>
        <p>Select or start dragging to use the ability on the enemy core.</p>
      </div>
    </div>
  `).addClass('clickthrough');
});

Screens.add('tutorial-finished', () => {
  const account = Session.getAccount();
  if (!account.tutorial) {
    request('app/complete-tutorial', false, () => {
      Analytics.event('Tutorial', 'finished');
      account.tutorial = 1;
      Session.setAccount(account);
    });
  }

  // Cleanup
  Event.fire('game over');
  Sound.play('win');
  GameServer.disconnect();
  GameServer.local(false);

  Screens.update('tutorial-finished', `
    <div class="tutorial-wrapper">
      <div class="tutorial-box clickcapture" style="width: 60%;">
        <h3>TUTORIAL FINISHED</h3>
        <p>
          Congratulations! You've finished the tutorial!
          Remember to check out the game guide for more infos and tips!
        </p>
        <button class="box" onclick="Screens.show('${Session.isOnboarding() ? 'onboarding-meta' : 'home'}')" style="margin-right: 1em">
          CONTINUE
        </button>
      </div>
    </div>
  `);
});
