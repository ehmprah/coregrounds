/**
 * @file TowerFactory.js
 * Holds the networked game data about the factory and handles user interaction
 */
import { pointToTile, networkablePosition } from '../../../Util/Geometry';
import Box from '../Box';
import Cooldown from '../../../Graphics/Elements/Cooldown';
import Input from '../../../Input';
import MoveOverlay from '../../../Graphics/Overlays/MoveOverlay';
import GameServer from '../../../Network/GameServer';
import Overlays from '../../../Graphics/Overlays/Overlays';
import Sound from '../../../Sound';
import UnitCache from '../../../Graphics/UnitCache';
import UserInterface from '../../../Interface/UserInterface';
import Config from '../../../Config/Config';
import ProgressBox from '../../../Graphics/Elements/ProgressBox';
import MinionFactory from './MinionFactory';
import InterfaceTips from '../../../Interface/InterfaceTips';

export default function TowerFactory(eid, index, playerIndex) {
  this.eid = eid;
  this.index = index;
  this.localIndex = 0;
  this.selected = false;
  this.player_id = playerIndex;
  this.box = new Box(32 + (index * 128), 924, 128, 128);
  this.box.dragStart = this.dragStart.bind(this);
  this.box.on('mouseup', this.select.bind(this));
  // Each tower factory has its own box for the entity on the map as well
  this.entityBox = new Box(-128, -128, 128, 128);
  this.entityBox.listening = false;
  this.entityBox.dragStart = this.dragStart.bind(this);
  this.entityBox.on('mouseup', (e) => {
    // If we're in spawn mode for a minion factory, we will not fire this
    if (UserInterface.selectedIndex() > 0) {
      const factory = UserInterface.selectedFactory();
      if (factory instanceof MinionFactory) return;
    }
    return this.select(e);
  });
  // Status FX
  this.effects = [];

  this.upgrade_1 = 0;
  this.upgrade_2 = 0;
  this.upgrade_3 = 0;
  this.upgradableStatNames = null;
  this.level_current = 0;
  this.level_progress = 0;
  this.entity_id = 0;
  this.cooldown_percentage = 0;
  this.cooldown_seconds = 0;

  const item = Config.query('factories', { id: this.eid });
  this.name = item.name.toUpperCase();
  this.hasTargetPreference = item.tower.default_target_preference !== undefined;
  this.targetTypes = {
    walls: 0,
    minions: 0,
    towers: 0,
    core: 0,
  };
  this.target = {
    high: 0,
    low: 1,
    first: 0,
    random: 0,
  };
}

TowerFactory.prototype.draw = function draw(context, tx, ty) {
  const x = tx || this.box.x;
  let y = ty || this.box.y;
  if (this.box.active && this.cooldown_percentage === 0) {
    y += 2;
  }

  // Route interface icon through the cooldown handler
  Cooldown.draw(
    context,
    x,
    y,
    this.eid,
    this.player_id,
    'Tower',
    'interface',
    this.cooldown_percentage,
    this.cooldown_seconds
  );

  ProgressBox.draw(
    context,
    x,
    y,
    this.level_current,
    this.level_progress,
    false,
    false
  );

  // Draw status effects
  if (this.effects.length) {
    context.statusFX(x, y, this.effects);
  }
};

TowerFactory.prototype.upgradeLowest = function upgradeLowest() {
  if (this.cooldown_seconds === 0) {
    let lowest = this.upgrade_1;
    let upgrade = 0;
    if (this.upgrade_2 < lowest) {
      lowest = this.upgrade_2;
      upgrade = 1;
    }
    if (this.upgrade_3 < lowest) {
      upgrade = 2;
    }
    GameServer.emit('UpgradeFactory', { index: this.index, upgrade });
    Sound.play('select');
  }
};

TowerFactory.prototype.select = function select(e) {
  InterfaceTips.hideTowerTip();

  if (e && e.altKey) {
    return this.upgradeLowest();
  }

  if (UserInterface.isDragging()) return;
  if (this.selected) {
    UserInterface.reset();
    Overlays.hide();
  } else {
    UserInterface.select(this.localIndex);
    Overlays.show('MoveOverlay', this);
  }
  Sound.play('select');
  // Prevent the event from bubbling further and into the factoryBar
  return false;
};

TowerFactory.prototype.dragStart = function dragStart() {
  InterfaceTips.hideTowerTip();
  if (UserInterface.isDragging()) return;
  UserInterface.dragStart(this.localIndex);
  Overlays.show('MoveOverlay', this);
};

TowerFactory.prototype.dragEnd = function dragEnd() {
  if (this.cooldown_percentage === 0) {
    this.action();
  } else {
    Sound.play('error');
  }
  UserInterface.reset();
  Overlays.hide();
};

TowerFactory.prototype.action = function action() {
  if (this.cooldown_seconds === 0) {
    const cursor = Input.position();
    const tile = pointToTile(cursor.x, cursor.y);
    if (MoveOverlay.validPosition(tile[0], tile[1])) {
      GameServer.emit('ActivateFactory', {
        index: this.index,
        position: networkablePosition(tile),
      });
      Sound.play('build_tower');
    }
  } else {
    Sound.play('error');
  }
  UserInterface.reset();
};

TowerFactory.prototype.drawDragger = function drawDragger(context) {
  const cursor = Input.position();
  context.drawImage(
    UnitCache.get(this.eid, this.player_id, 'Tower', 'drag'),
    cursor.x - 96,
    cursor.y - 96,
    192,
    192
  );
};
