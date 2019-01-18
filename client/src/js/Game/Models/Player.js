/**
 * @file
 * Player object
 */

function Player() {
  this.name = '';
  this.icon = 2000;
  this.color = 2400;
  this.teamId = 0;
  this.pickable = [];
  this.banned_factory = 0;
  this.banned_ability = 0;
  this.factories = [];
  this.minion_limit = 0;
  this.globalTarget = 0;
  this.settled = 0;
  this.pid = 0;
  this.skins = {};
}

export default Player;
