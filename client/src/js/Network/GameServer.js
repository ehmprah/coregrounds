/**
 * @file
 * Networking interface for the game server
 * @see https://en.wikipedia.org/wiki/Radian#/media/File:Degree-Radian_Conversion.svg
 *
 * @todo REWRITE TO ES6
 * @todo PERFORMANCE!!!
 * @todo split this file into subfiles
 * @todo reuse variables properly, define those once, don't stress the GC here
 * @todo define vars at beginning of entity loop, don't declare within!!!
 * @todo optimize reconnection by adding a network stack in case we miss the first update
 * TODO round all coordinates because performance!!!
 */
import { getX, getY, getAngle } from '../Util/Geometry';
import AbilityFactory from '../Game/Models/Factories/AbilityFactory';
import Analytics from '../Analytics';
import BuildOverlay from '../Graphics/Overlays/BuildOverlay';
import Config from '../Config/Config';
import Emote from '../Graphics/Elements/Emote';
import EntityPool from '../Game/EntityPool';
import Event from '../Event';
import FogOfWar from '../Graphics/Elements/FogOfWar';
import Game from '../Game/Game';
import MinionFactory from '../Game/Models/Factories/MinionFactory';
import Particles from '../Graphics/Particles/Particles';
import Picks from '../Graphics/Elements/Picks';
import Player from '../Game/Models/Player';
import GameServerSocket from './GameServerSocket';
import Sound from '../Sound';
import TowerFactory from '../Game/Models/Factories/TowerFactory';
import Visuals from '../Graphics/Visuals/Visuals';
import WallFactory from '../Game/Models/Factories/WallFactory';
import Graphics from '../Graphics/Graphics';

let local;

export default {

  isConnected() {
    return local || GameServerSocket.isConnected();
  },

  local(handler) {
    local = handler;
  },

  connect(ip, token) {
    if (Config.User.get('game-server')) {
      GameServerSocket.connect(`ws://${Config.User.get('game-server')}`, token);
    } else {
      GameServerSocket.connect(`ws://${ip.replace('::ffff:', '')}:${Config.PORT}`, token);
    }
  },

  disconnect() {
    if (local) {
      local.close();
    } else {
      GameServerSocket.close();
    }
  },

  emit(key, data) {
    if (local) {
      local.emit(key, data);
    } else {
      GameServerSocket.emit(key, data);
    }
  },
};

export function tick(payload) {
  const { state } = Game;
  let phase;

  // TODO get rid of the object here, use the payload and its respective getters

  const match = payload.toObject();
  const gphase = payload.getPhaseCase();

  // Log traffic if toggled
  if (Config.User.get('network-logging')) {
    // eslint-disable-next-line
    console.log(match);
  }

  // Things to do during the first network update
  if (!state.players.length) {
    state.players.push(new Player());
    state.players.push(new Player());

    let m = match.pick;
    if (!match.pick) m = match.game;

    // Set playerIndex
    if (m && m.playerindex) {
      if ((1 << 31) & m.playerindex) {
        Game.playerIndex = m.playerindex ^ (1 << 31);
      }
    }

    // Get networked colors
    if (m && m.playerList) {
      m.playerList.forEach((player, index) => {
        if ((1 << 31) & player.colorid) {
          state.players[index].color = player.colorid ^ (1 << 31);
        }
      });
    }

    // Track game start
    Analytics.event('Game', 'start');
    // We have to update the colors first before everything else fires their
    // caching mechanisms on the game start.
    Event.fire('match colors updated', 'game started', 'game ready');
  }

  if (match.pick) {
    if (Game.status === 0) {
      Event.fire('before draft phase update');
    }

    const update = match.pick;

    // Set remaining pick time
    if (update.time) {
      // Set match time
      if (update.time >= 10) {
        const newTime = (update.time / 10) | 0;
        if (newTime < state.time && newTime <= 5 && Game.state.draft < 5) {
          if (Game.activePlayerIndex === Game.playerIndex) {
            Sound.play('countdown');
          }
        }
        state.time = newTime;
      } else {
        state.time = update.time / 10;
      }
    }

    // Handle incoming picks and bans
    if (update.playerList && update.playerList.length) {
      updatePicks(update);
    }
  }

  if (match.game) {
    const update = match.game;

    // Set match time
    const newTime = (update.gametime / 1000) | 0;
    if (newTime > state.time) {
      Event.fire('slow network update');
    }
    state.time = newTime;

    if (update.playerList) {
      if (update.playerList.length) {
        updateEntities(update, payload);
      }
    }
  }

  if (match.finalize) {
    match.finalize.playersList.forEach((player, playerIndex) => {
      const icon = removeDirtyFlag(player.icon);
      if (icon) {
        Game.state.players[playerIndex].icon = icon;
      }
      if (player.name && player.name.isset) {
        Game.state.players[playerIndex].name = player.name.text;
      }
      if (player.teamid) {
        Game.state.players[playerIndex].teamId = player.teamid;
      }
      if (player.emoteid) {
        Emote.show(playerIndex, player.emoteid);
      }
    });
    if (match.finalize.winningteamid) {
      Game.winningTeamId = removeDirtyFlag(match.finalize.winningteamid);
      if (Game.winningTeamId === Game.state.players[Game.playerIndex].teamId) {
        Sound.play('win');
      } else {
        Sound.play('loss');
      }
    }
    if (Game.status === 2) {
      Game.status = 3;
      Event.fire('finalize phase');
    }
    if (match.finalize.permmatchid) {
      Game.finished = true;
      Game.matchId = match.finalize.permmatchid;
    }
    if (!match.finalize.timeleft) {
      Game.finished = true;
      GameServerSocket.close();
      Event.fire('game finished');
    }
  }

  if (gphase === 1) {
    if (Game.status === 0) {
      Game.status = 1;
      Event.fire('draft phase', 'status changed');
    }
    Game.status = 1;
    phase = 'draft';
  }

  if (gphase === 2) {
    if (Game.status < 2) {
      Game.status = 2;
      Event.fire('game phase', 'status changed');
    }
    Game.status = 2;
    phase = 'game';

    // Update warmup time
    if (match.game.warmuptime) {
      Game.state.warmup = match.game.warmuptime;
      if (Game.state.warmup % 10 === 0) Sound.play('countdown');
    } else if (Game.state.warmup) {
      Game.state.warmup = 0;
      Sound.play('countdown');
    }

    // We check this here so we fire it after reconnecting to a running match
    if (Game.state.warmup === 0 && !Game.state.warmup_finished) {
      Game.state.warmup_finished = true;
      Event.fire('warmup finished');
    }
  }

  // We fire the network update event at the end of the update loop so
  // everyone else has newest data to work with
  Event.fire(`network update ${phase}`, 'network update');
}

function updatePicks(data) {
  let fireEvent = false;
  // Keep track of the pick phase sub state
  if (data.state) {
    if (data.state !== Game.state.draft) {
      Event.fire('draft subphase changed');
    }
    if (data.state === 5 && Game.state.draft === 4) {
      Event.fire('picks finished');
    }
    Game.state.draft = data.state;
  }
  if ((1 << 31) & data.activeplayerindex) {
    Game.activePlayerIndex = data.activeplayerindex ^ (1 << 31);
    Event.fire('draft state changed');
  }
  data.playerList.forEach((player, playerIndex) => {
    // Update skins
    if (player.skins && player.skins.isset && player.skins.idsList.length) {
      player.skins.idsList.forEach((id) => {
        const item = Config.query('skins', { id });
        if (item) Game.state.players[playerIndex].skins[item.unitId] = id;
      });
    }
    // Update modpage id
    if (player.modpageid) {
      Game.state.players[playerIndex].pid = player.modpageid;
    }
    // Show emotes
    if (player.emoteid) {
      Emote.show(playerIndex, player.emoteid);
    }
    // Update picks
    player.factorybansList.forEach((id, index) => {
      Picks.ban(id, playerIndex, index);
      fireEvent = true;
    });
    player.abilitybansList.forEach((id, index) => {
      Picks.ban(id, playerIndex, index);
      fireEvent = true;
    });
    player.factorypicksList.forEach((id, index) => {
      Picks.pick(id, playerIndex, index);
      fireEvent = true;
    });
    player.abilitypicksList.forEach((id, index) => {
      Picks.pick(id, playerIndex, index);
      fireEvent = true;
    });
    // Update available picks
    if (player.availableidsList && player.availableidsList.length) {
      Game.state.players[playerIndex].pickable = player.availableidsList.slice();
      fireEvent = true;
    }
  });
  if (fireEvent) {
    Event.fire('updated available picks');
  }
}

function updateAbilities(state, updated) {
  const { factories } = state.players[Game.playerIndex];
  updated.forEach((ability, index) => {
    // If we have an id, this is the first transmission, so we create a new factory
    const id = removeDirtyFlag(ability.id);
    if (id) factories.push(new AbilityFactory(id, index + 6, Game.playerIndex));
    // We update always
    updateAbility(factories[index + 6], ability);
  });
}

function updateFactories(state, updated) {
  const { factories } = state.players[Game.playerIndex];

  if (factories.length < 6) {
    // Create factories
    let i;
    let eid;
    for (i = 0; i < updated.length; i++) {
      if (updated[i].wall) {
        factories.push(new WallFactory(i, Game.playerIndex));
      }
      if (updated[i].tower) {
        eid = updated[i].id ^ (1 << 31);
        if (eid) { // @todo add proper error handling here!
          factories.push(new TowerFactory(eid, i, Game.playerIndex));
          factories[i].upgradableStatNames = getUpgradableStatNames(eid);
        }
      }
      if (updated[i].minion) {
        eid = updated[i].id ^ (1 << 31);
        if (eid) { // @todo add proper error handling here!
          factories.push(new MinionFactory(eid, i, Game.playerIndex));
          factories[i].upgradableStatNames = getUpgradableStatNames(eid);
        }
      }
    }
    // Sort them
    factories.sort((a, b) => {
      if (a instanceof WallFactory && b instanceof TowerFactory) {
        return -1;
      }
      if (a instanceof TowerFactory && b instanceof MinionFactory) {
        return -1;
      }
      if (a instanceof MinionFactory && !(b instanceof MinionFactory)) {
        return 1;
      }
      return 0;
    });
    // Set local index and update input box location
    factories.forEach((item, index) => {
      if (index < 6) { // exclude abilities
        item.localIndex = index;
        item.box.x = 32 + (index * 128);
      }
    });
  }
  // Update the factories
  factories.forEach((item, index) => {
    if (index < 6) { // exclude abilities
      // We update the local factory with the fitting remote one
      updateFactory(item, updated[item.index]);
    }
  });
}

function updateEntities(update, payload) {
  const state = Game.state;
  // Keep in check which entities got an update and which were left
  // untouched, so we can GC them.
  let touched = {};
  let entity;
  let guid;
  let e;

  update.playerList.forEach((player, playerIndex) => {
    if (player.flags) {
      if ((player.flags ^ (1 << 31)) === 1) {
        state.players[playerIndex].settled = 1;
      }
    }

    if (player.skins && player.skins.isset && player.skins.idsList.length) {
      player.skins.idsList.forEach((id) => {
        const item = Config.query('skins', { id });
        if (item) state.players[playerIndex].skins[item.unitId] = id;
      });
    }

    if (player.factoriesList.length) {
      updateFactories(state, player.factoriesList);
    }

    if (player.abilitiesList.length) {
      updateAbilities(state, player.abilitiesList);
    }

    if (player.buildingmap) {
      BuildOverlay.update(payload.getGame().getPlayerList()[playerIndex].getBuildingmap_asU8());
    }

    if (player.visibilitymap) {
      FogOfWar.update(payload.getGame().getPlayerList()[playerIndex].getVisibilitymap_asU8());
    }

    if (player.globaltarget != null) {
      if (player.globaltarget.isset) {
        state.players[playerIndex].globalTarget = player.globaltarget.guid;
      } else {
        state.players[playerIndex].globalTarget = 0;
      }
    }

    if (player.emoteid) {
      // Spawn new emote for this player
      Emote.show(playerIndex, player.emoteid);
    }

    player.visualsList.forEach((visual) => {
      Visuals.show(visual.id, playerIndex, getX(visual.position), getY(visual.position));
    });

    // Minion limit
    if ((1 << 31) & player.minionlimit) {
      state.players[playerIndex].minion_limit = (player.minionlimit ^ (1 << 31)) / 100;
    }

    // @todo rework the following into a single for loop with variable
    // keys and the init() callback

    // Core
    if (player.core) {
      entity = player.core;
      guid = entity.unit.guid;
      touched[guid] = 1;
      if (state.entities[guid] === undefined) {
        state.entities[guid] = EntityPool.get('Core');
        state.entities[guid].player_id = playerIndex;
        state.entities[guid].entity_id = guid;
        state.entities[guid].eid = entity.unit.id ^ (1 << 31);
        state.entities[guid].init(); // @todo rewrite this to a single call passing on all the entity values!
      }
      updateEntity(state.entities[guid], entity);
    }

    e = player.wallsList.length;
    while (e--) {
      entity = player.wallsList[e];
      guid = entity.unit.guid;
      touched[guid] = 1;
      if (state.entities[guid] === undefined) {
        // Skip this unit if it's just the "deathrattle" transmission
        if (!entity.unit.id) continue;
        // Otherwise create a new unit
        state.entities[guid] = EntityPool.get('Wall');
        state.entities[guid].player_id = playerIndex;
        state.entities[guid].entity_id = guid;
        state.entities[guid].eid = entity.unit.id ^ (1 << 31);
        state.entities[guid].init();
        // We play the wall build sound here, not when issuing the
        // command, because the tile could be contested by both players.
        if (playerIndex === Game.playerIndex) {
          Sound.play('build_wall');
        }
      }
      updateEntity(state.entities[guid], entity, playerIndex);
    }

    // Minions
    e = player.minionsList.length;
    while (e--) {
      entity = player.minionsList[e];
      guid = entity.unit.guid;
      touched[guid] = 1;
      if (state.entities[guid] === undefined) {
        // Skip this unit if it's just the "deathrattle" transmission
        if (!entity.unit.id) continue;
        // Otherwise create a new unit
        state.entities[guid] = EntityPool.get('Minion');
        state.entities[guid].player_id = playerIndex;
        state.entities[guid].entity_id = guid;
        state.entities[guid].eid = entity.unit.id ^ (1 << 31);
        state.entities[guid].init();
      }
      updateEntity(state.entities[guid], entity);
    }

    // Towers
    e = player.towersList.length;
    while (e--) {
      entity = player.towersList[e];
      guid = entity.unit.guid;
      touched[guid] = 1;
      if (state.entities[guid] === undefined) {
        // Skip this unit if it's just the "deathrattle" transmission
        if (!entity.unit.id) continue;
        // Otherwise create a new unit
        state.entities[guid] = EntityPool.get('Tower');
        state.entities[guid].player_id = playerIndex;
        state.entities[guid].entity_id = guid;
        state.entities[guid].eid = entity.unit.id ^ (1 << 31);
        state.entities[guid].init();
      }
      updateEntity(state.entities[guid], entity);
    }

    // Projectiles
    e = player.projectilesList.length;
    while (e--) {
      entity = player.projectilesList[e];
      guid = entity.unit.guid;
      touched[guid] = 1;
      if (state.entities[guid] === undefined) {
        // Skip this unit if it's just the "deathrattle" transmission
        if (!entity.unit.id) continue;
        // Otherwise create a new unit
        state.entities[guid] = EntityPool.get('Projectile');
        state.entities[guid].player_id = playerIndex;
        state.entities[guid].entity_id = guid;
        state.entities[guid].id = entity.unit.id ^ (1 << 31);
        state.entities[guid].init();
      }
      updateEntity(state.entities[entity.unit.guid], entity);
    }

    // Triggers
    e = player.triggersList.length;
    while (e--) {
      entity = player.triggersList[e];
      guid = entity.unit.guid;
      touched[guid] = 1;
      if (state.entities[guid] === undefined) {
        // Skip this unit if it's just the "deathrattle" transmission
        if (!entity.unit.id) continue;
        // Otherwise create a new unit
        state.entities[guid] = EntityPool.get('Trigger');
        state.entities[guid].player_id = playerIndex;
        state.entities[guid].entity_id = guid;
        state.entities[guid].id = entity.unit.id ^ (1 << 31);
        state.entities[guid].init();
      }
      updateEntity(state.entities[entity.unit.guid], entity);
    }

    // We update the foremost minion last to make sure it exists already
    if (player.foremostminion != null) {
      if (player.foremostminion.isset) {
        // Reset previous foremost minoin
        Object.keys(state.entities).forEach((id) => {
          if (state.entities[id].foremost) state.entities[id].foremost = false;
        });
        // Set it for this one
        state.entities[player.foremostminion.guid].foremost = true;
      } else {
        Object.keys(state.entities).forEach((id) => {
          if (state.entities[id].foremost) state.entities[id].foremost = false;
        });
      }
    }
  });

  // Garbage collect all entitites which weren't touched by this update or
  // have fallen below zero health during this update
  // eslint-disable-next-line
  for (guid in state.entities) {
    entity = state.entities[guid];
    // We don't remove cores from the game
    if (entity.type !== 'Core') {
      if (entity.state === 2) {
        killEntity(entity, guid);
      } else if (touched[guid] === undefined) {
        removeEntity(entity, guid);
      }
    }
  }
}

function removeEntity(entity, guid) {
  // Remove the entity from the game
  delete Game.state.entities[guid];
  // And give it back to the EntityPool
  EntityPool.free(entity.type, entity);
}

function killEntity(entity, guid) {
  const { type } = entity;
  const owner = entity.player_id === Game.playerIndex
    ? 'own'
    : 'enemy';

  // For own towers, we have to reset the corresponding factory as well.
  if (entity.type === 'Tower' && entity.player_id === Game.playerIndex) {
    Game.state.players[Game.playerIndex].factories.forEach((factory, index) => {
      if (factory.entity_id === entity.entity_id) {
        Game.state.players[Game.playerIndex].factories[index].entity_id = 0;
        Game.state.players[Game.playerIndex].factories[index].entityBox.listening = false;
      }
    });
  }

  // Everything but projectiles and triggers gets a sound and a particle effekt
  if (type !== 'Projectile' && type !== 'Trigger') {
    Sound.play('explosion');
    // We create the explosion based on a copy of this entity; passing the
    // actual entity here will result in a fucked up entity loop.
    // TODO investigate and find solution later, this is good enough for now.
    // Particles.spriteExplosion(entity);
    Particles.spriteExplosion(JSON.parse(JSON.stringify(entity)));
  }

  // Tell the event system about the dead entity
  Event.fire(`${owner} ${type} killed`);

  removeEntity(entity, guid);
}

function updateEntity(entity, update, playerIndex) {

  // DEBUG
  /* eslint-disable */
  if (Config.User.get('debug-stats')) {
    if (update.unit.statsList) {
      entity.stats = [];
      let i = update.unit.statsList.length;
      while (i--) {
        if (update.unit.statsList[i]) {
          if (i === 0) entity.stats.push('ARM: ' + update.unit.statsList[i].toFixed(2));
          if (i === 1) entity.stats.push('APN: ' + update.unit.statsList[i].toFixed(2));
          if (i === 2) entity.stats.push('APS: ' + update.unit.statsList[i].toFixed(2));
          if (i === 3) entity.stats.push('DMG: ' + update.unit.statsList[i].toFixed(2));
          if (i === 4) entity.stats.push('TKN: ' + update.unit.statsList[i].toFixed(2));
          if (i === 5) entity.stats.push('HLG: ' + update.unit.statsList[i].toFixed(2));
          if (i === 6) entity.stats.push('MHP: ' + update.unit.statsList[i].toFixed(2));
          if (i === 7) entity.stats.push('MSP: ' + update.unit.statsList[i].toFixed(2));
          if (i === 8) entity.stats.push('RNG: ' + update.unit.statsList[i].toFixed(2));
          if (i === 9) entity.stats.push('REG: ' + update.unit.statsList[i].toFixed(2));
          if (i === 10) entity.stats.push('THR: ' + update.unit.statsList[i].toFixed(2));
        }
      }
    }
  }
  /* eslint-enable */


  // Update the playerIndex in case that's changed (WallSwap!)
  if (playerIndex !== undefined) {
    entity.player_id = playerIndex;
  }

  // Dirty flag: x > 2147483648 = update!

  if ((1 << 31) & update.unit.hp) {
    const old = entity.health_percentage;
    // Remove update flag from value
    update.unit.hp ^= 1 << 31;
    // Update health percentage
    entity.health_percentage = update.unit.hp / 100;
    if (entity.health_percentage > 0) {
      Particles.onHit(entity.x, entity.y, entity.player_id, old - entity.health_percentage);
      if (entity.type === 'Core' &&
          entity.health_percentage < 1 &&
          old > entity.health_percentage &&
          entity.player_id === Game.playerIndex) {
        Graphics.screenshake();
      }
    }
  }

  if ((1 << 31) & update.unit.state) {
    // Remove update flag from value
    update.unit.state ^= 1 << 31;
    // Update health percentage
    entity.state = update.unit.state;
  }

  if ((1 << 31) & update.unit.position) {
    // If the tower is about to move, spawn dust at the origin
    if (entity.type === 'Tower') {
      if (entity.x || entity.y) {
        Particles.dust(entity.x, entity.y, entity.player_id);
      }
    }

    // Remove update flag from value
    update.unit.position ^= 1 << 31;

    entity.x = getX(update.unit.position) - 64;
    entity.y = getY(update.unit.position) - 64;

    // Towers need to update their factory's entityBox
    if (entity.type === 'Tower' && entity.player_id === Game.playerIndex) {
      Game.state.players[Game.playerIndex].factories.forEach((factory) => {
        if (factory.entity_id === entity.entity_id) {
          factory.entityBox.x = entity.x;
          factory.entityBox.y = entity.y;
          factory.entityBox.listening = true;
        }
      });
    }
  }

  // Aura visual ids
  if (update.unit.auravisualids) {
    if (update.unit.auravisualids.isset) {
      if (update.unit.auravisualids.idsList.length) {
        entity.effects = [];
        update.unit.auravisualids.idsList.forEach((id) => {
          entity.effects.push(id);
        });
      } else {
        entity.effects = [];
      }
    }
  }

  // Start position for projectiles
  if (update.startposition && (1 << 31) & update.startposition) {
    // Remove update flag from value
    update.startposition ^= 1 << 31;

    const first = entity.start_x === 0 && entity.start_y === 0;

    // Update the position
    entity.start_x = getX(update.startposition) - 64;
    entity.start_y = getY(update.startposition) - 64;

    // Tell the projectile to play its sound (and wether this is the first time)
    entity.sound(first);
  }

  // Target position for projectiles (we check this differently here)
  if (update.destinationposition && update.destinationposition > 2147483648) {
    // Remove update flag from value
    update.destinationposition ^= 1 << 31;
    // Update the position
    entity.target_x = getX(update.destinationposition) - 64;
    entity.target_y = getY(update.destinationposition) - 64;
  }

  // if (entity.type === 'Projectile') {
  //   console.log(
  //     entity.x,
  //     entity.y,
  //     entity.start_x,
  //     entity.start_y,
  //     entity.target_x,
  //     entity.target_y
  //   );
  // }

  if ((1 << 31) & update.attackprogress) {
    // Remove update flag from value
    update.attackprogress ^= 1 << 31;
    // Update the value, doing calculations once
    entity.attack_percentage = update.attackprogress / 100;
  }

  if ((1 << 31) & update.attackradius) {
    // Remove update flag from value
    update.attackradius ^= 1 << 31;
    // Update the value, doing calculations once
    entity.attackRadius = update.attackradius / 1000;
  }

  if ((1 << 31) & update.chainbonus) {
    // Remove update flag from value
    update.chainbonus ^= 1 << 31;
    entity.chainBonus = update.chainbonus;
  }

  if ((1 << 31) & update.cooldown) {
    // Remove update flag from value
    update.cooldown ^= 1 << 31;
    // Set new values
    entity.cooldown_seconds = (update.cooldown & 32767) / 10;
    entity.cooldown_percentage = (update.cooldown >> 16) / 100;
  }

  if ((1 << 31) & update.directionvector) {
    // Remove update flag from value
    update.directionvector ^= 1 << 31;

    // Set new values
    entity.vector_x = ((update.directionvector & 32767) / 1000) - 1;
    entity.vector_y = ((update.directionvector >> 15) / 1000) - 1;

    // Radians starts to the right, at 15:00 h
    entity.angle = getAngle(Math.atan2(entity.vector_y, entity.vector_x));
  }
}

function updateFactory(factory, update) {
  if (update.wall) {
    if ((1 << 31) & update.wall.stacks) {
      // Remove update flag from value
      update.wall.stacks ^= 1 << 31;
      factory.stacks_current = update.wall.stacks & 127;
      factory.stacks_max = (update.wall.stacks >> 7) & 127;
      factory.stacks_progress = (update.wall.stacks >> 14) / 100;
      if (factory.stacks_current === factory.stacks_max) {
        Sound.play('full_stack');
      }
    }
  }

  if (update.tower) {
    // Entity id
    if (update.tower.guid) {
      factory.entity_id = update.tower.guid;
    }

    // Cooldowns
    if ((1 << 31) & update.tower.cooldown) {
      // Remove update flag from value
      update.tower.cooldown ^= 1 << 31;

      // Set new values
      factory.cooldown_seconds = (update.tower.cooldown & 32767) / 10;
      factory.cooldown_percentage = (update.tower.cooldown >> 16) / 100;
    }

    // Upgrades
    if ((1 << 31) & update.tower.upgrades) {
      // Remove update flag from value
      update.tower.upgrades ^= 1 << 31;
      factory.upgrade_1 = update.tower.upgrades & 7;
      factory.upgrade_2 = (update.tower.upgrades >> 3) & 7;
      factory.upgrade_3 = update.tower.upgrades >> 6;
      Sound.play('upgrade_complete');
    }

    // XP
    if ((1 << 31) & update.tower.level) {
      // Remove update flag from value
      update.tower.level ^= 1 << 31;
      // Set new values
      factory.level_current = update.tower.level & 32767;
      factory.level_progress = (update.tower.level >> 15) / 100;
      if (factory.level_current === 20) {
        factory.level_progress = 0;
      }
    }

    // Target Preference
    if ((1 << 31) & update.tower.targetpreference) {
      // Remove update flag from value
      update.tower.targetpreference ^= 1 << 31;

      const flags = update.tower.targetpreference & 32767;
      const target = update.tower.targetpreference >> 15;

      factory.targetTypes.core = flags & 1;
      factory.targetTypes.walls = (flags >> 1) & 1;
      factory.targetTypes.towers = (flags >> 2) & 1;
      factory.targetTypes.minions = (flags >> 3) & 1;

      factory.target.low = target === 1 ? 1 : 0;
      factory.target.high = target === 2 ? 1 : 0;
      factory.target.first = target === 3 ? 1 : 0;
      factory.target.random = target === 4 ? 1 : 0;

      // console.log(`
      //   factory: ${factory.eid},
      //   core: ${factory.targetTypes.core}
      //   walls: ${factory.targetTypes.walls}
      //   towers: ${factory.targetTypes.towers}
      //   minions: ${factory.targetTypes.minions}
      //   low: ${factory.target.low}
      //   high: ${factory.target.high}
      //   first: ${factory.target.first}
      //   random: ${factory.target.random}
      // `);
    }
  }

  // Minion specific
  if (update.minion) {
    // Cooldowns
    if ((1 << 31) & update.minion.cooldown) {
      // Remove update flag from value
      update.minion.cooldown ^= 1 << 31;

      // Set new values
      factory.cooldown_seconds = (update.minion.cooldown & 32767) / 10;
      factory.cooldown_percentage = (update.minion.cooldown >> 16) / 100;
    }

    // Upgrades
    if ((1 << 31) & update.minion.upgrades) {
      // Remove update flag from value
      update.minion.upgrades ^= 1 << 31;
      factory.upgrade_1 = update.minion.upgrades & 7;
      factory.upgrade_2 = (update.minion.upgrades >> 3) & 7;
      factory.upgrade_3 = update.minion.upgrades >> 6;
      Sound.play('upgrade_complete');
    }

    // XP
    if ((1 << 31) & update.minion.level) {
      // Remove update flag from value
      update.minion.level ^= 1 << 31;
      // Set new values
      factory.level_current = update.minion.level & 32767;
      factory.level_progress = (update.minion.level >> 15) / 100;
      if (factory.level_current === 20) {
        factory.level_progress = 0;
      }
    }

    // Stacks
    if ((1 << 31) & update.minion.stacks) {
      // Remove update flag from value
      update.minion.stacks ^= 1 << 31;
      factory.stacks_current = update.minion.stacks & 127;
      factory.stacks_max = (update.minion.stacks >> 7) & 127;
      factory.stacks_progress = (update.minion.stacks >> 14) / 100;
      if (factory.stacks_current === factory.stacks_max) {
        Sound.play('full_stack');
      }
    }

    // Target Preference
    if ((1 << 31) & update.minion.targetpreference) {
      // Remove update flag from value
      update.minion.targetpreference ^= 1 << 31;

      const flags = update.minion.targetpreference & 32767;
      const target = update.minion.targetpreference >> 15;

      factory.targetTypes.core = flags & 1;
      factory.targetTypes.walls = (flags >> 1) & 1;
      factory.targetTypes.towers = (flags >> 2) & 1;
      factory.targetTypes.minions = (flags >> 3) & 1;

      factory.target.low = target === 1 ? 1 : 0;
      factory.target.high = target === 2 ? 1 : 0;
      factory.target.first = target === 3 ? 1 : 0;
      factory.target.random = target === 4 ? 1 : 0;
    }
  }

  // Aura visual ids
  if (update.auravisualids) {
    if (update.auravisualids.isset) {
      if (update.auravisualids.idsList.length) {
        factory.effects = [];
        update.auravisualids.idsList.forEach((id) => {
          factory.effects.push(id);
        });
      } else {
        factory.effects = [];
      }
    }
  }
}

function updateAbility(ability, update) {
  const cooldown = removeDirtyFlag(update.cooldown);
  if (cooldown) {
    ability.cooldown_seconds = (cooldown & 32767) / 10;
    ability.cooldown_percentage = (cooldown >> 16) / 100;
  }
}

// TODO this should be in the factory's prototype
export function getUpgradableStatNames(id) {
  let type, upgrades = [], item = Config.query('factories', { id });
  // Determine type
  if (item.minion !== undefined) type = 'minion';
  if (item.tower !== undefined) type = 'tower';
  // Get names
  upgrades.push(item[type].upgrades[0].name.toUpperCase());
  upgrades.push(item[type].upgrades[1].name.toUpperCase());
  upgrades.push(item[type].upgrades[2].name.toUpperCase());
  // And return the array
  return upgrades;
}

function removeDirtyFlag(value) {
  return value
    ? value ^ (1 << 31)
    : value;
}
