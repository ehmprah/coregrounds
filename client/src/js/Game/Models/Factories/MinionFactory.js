/**
 * @file
 * Holds the networked game data about the factory and handles user interaction
 */
import Box from '../Box';
import Config from '../../../Config/Config';
import Cooldown from '../../../Graphics/Elements/Cooldown';
import Game from '../../../Game/Game';
import GameServer from '../../../Network/GameServer';
import Input from '../../../Input';
import Overlays from '../../../Graphics/Overlays/Overlays';
import ProgressBox from '../../../Graphics/Elements/ProgressBox';
import Sound from '../../../Sound';
import UnitCache from '../../../Graphics/UnitCache';
import UserInterface from '../../../Interface/UserInterface';
import Visuals from '../../../Graphics/Visuals/Visuals';
import InterfaceTips from '../../../Interface/InterfaceTips';

export default function MinionFactory(eid, index, playerIndex) {
  this.eid = eid;

  this.effects = [];

  this.upgrade_1 = 0;
  this.upgrade_2 = 0;
  this.upgrade_3 = 0;

  this.upgradableStatNames = null;

  this.level_current = 0;
  this.level_progress = 0;

  this.stacks_current = 0;
  this.stacks_max = 0;
  this.stacks_progress = 0;

  this.cooldown_percentage = 0;
  this.cooldown_seconds = 0;


  const item = Config.query('factories', { id: this.eid });
  this.name = item.name.toUpperCase();
  this.hasTargetPreference = item.minion.default_target_preference !== undefined;
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

  // this = factory;
  this.index = index;
  this.localIndex = 0;
  this.selected = false;
  this.player_id = playerIndex;
  this.box = new Box(32 + (index * 128), 924, 128, 128, true);
  this.box.dragStart = this.dragStart.bind(this);
  this.box.on('mouseup', this.select.bind(this));
}

MinionFactory.prototype.draw = function draw(context, tx, ty) {
  const x = tx || this.box.x;
  let y = ty || this.box.y;
  if (this.box.active && this.cooldown_seconds === 0) {
    y += 2;
  }

  // When at the minion limit, we show factories with the red tint
  const prefix = Game.state.players[this.player_id].minion_limit === 1
    ? 'interface_limit'
    : 'interface';

  // Route interface icon through the cooldown handler
  Cooldown.draw(
    context,
    x,
    y,
    this.eid,
    this.player_id,
    'Minion',
    prefix,
    this.cooldown_percentage,
    this.cooldown_seconds
  );

  ProgressBox.draw(
    context,
    x,
    y,
    this.level_current,
    this.level_progress,
    this.stacks_current,
    this.stacks_progress,
    this.stacks_max
  );

  // Draw status effects
  if (this.effects.length) {
    context.statusFX(x, y, this.effects);
  }
};

MinionFactory.prototype.dragStart = function dragStart() {
  InterfaceTips.hideMinionTip();
  UserInterface.dragStart(this.localIndex);
};

MinionFactory.prototype.dragEnd = function dragEnd() {
  if (this.cooldown_percentage === 0 && this.stacks_current > 0) {
    this.action();
  } else {
    Sound.play('error');
  }
};

MinionFactory.prototype.upgradeLowest = function upgradeLowest() {
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

MinionFactory.prototype.select = function select(e) {
  InterfaceTips.hideMinionTip();

  if (e) {
    if (e.ctrlKey || e.shiftKey) {
      return this.action();
    }

    if (e.altKey) {
      return this.upgradeLowest();
    }
  }

  if (this.selected) {
    UserInterface.reset();
    Overlays.hide();
  } else {
    UserInterface.select(this.localIndex);
    Overlays.show('SpawnOverlay');
  }
  Sound.play('select');
  // Prevent the event from bubbling further and into the factoryBar
  return false;
};

MinionFactory.prototype.action = function action() {
  if (this.cooldown_seconds === 0 && this.stacks_current > 0) {
    GameServer.emit('ActivateFactory', {
      index: this.index,
      position: 0,
    });
    Sound.play('spawn_minion');
    Visuals.unitDrop(this.eid);
  } else {
    Sound.play('error');
    UserInterface.reset();
  }
};

MinionFactory.prototype.drawDragger = function drawDragger(context) {
  const cursor = Input.position();
  context.drawImage(
    UnitCache.get(this.eid, this.player_id, 'Minion', 'drag'),
    cursor.x - 96,
    cursor.y - 96,
    192,
    192
  );
};
