/**
 * AbilityFactory.js
 * Holds the networked game data about a factory and handles user interaction
 */
import Box from '../Box';
import Game from '../../Game';
import Input from '../../../Input';
import Sound from '../../../Sound';
import GameServer from '../../../Network/GameServer';
import UserInterface from '../../../Interface/UserInterface';
import Cache from '../../../Graphics/Cache';
import UnitCache from '../../../Graphics/UnitCache';
import Visuals from '../../../Graphics/Visuals/Visuals';
import ProgressBox from '../../../Graphics/Elements/ProgressBox';
import Overlays from '../../../Graphics/Overlays/Overlays';
import BuildOverlay from '../../../Graphics/Overlays/BuildOverlay';
import { pointToTile, networkablePosition } from '../../../Util/Geometry';
import InterfaceTips from '../../../Interface/InterfaceTips';

export default function WallFactory(index, playerIndex) {
  this.stacks_current = 0;
  this.stacks_max = 0;
  this.stacks_progress = 0;

  this.effects = [];

  this.index = index;
  this.name = 'WALL';
  this.localIndex = 0;
  this.selected = false;
  this.player_id = playerIndex;
  this.box = new Box(32, 924, 128, 128);
  this.box.dragStart = this.dragStart.bind(this);
  this.box.on('mouseup', this.select.bind(this));
}

WallFactory.prototype.draw = function draw(context, tx, ty) {
  const x = tx || this.box.x;
  let y = ty || this.box.y;
  if (this.box.active && this.stacks_current) {
    y += 2;
  }

  if (this.selected) {
    context.drawImage(UnitCache.getHighlight(1, this.player_id, 'Wall'), x, y);
  } else {
    context.drawImage(UnitCache.get(1, this.player_id, 'Wall'), x, y);
  }

  ProgressBox.draw(
    context,
    x,
    y,
    false,
    false,
    this.stacks_current,
    this.stacks_progress,
    this.stacks_max
  );

  // Draw status effects
  if (this.effects.length) {
    context.statusFX(x, y, this.effects);
  }
};

WallFactory.prototype.dragStart = function dragStart() {
  InterfaceTips.hideWallTip();
  UserInterface.select(this.localIndex);
  UserInterface.dragStart(this.localIndex);
  Overlays.show('BuildOverlay');
};

WallFactory.prototype.dragEnd = function dragEnd() {
  if (this.stacks_current > 0) {
    this.action();
  } else {
    Sound.play('error');
  }
  UserInterface.reset();
  Overlays.hide();
};

WallFactory.prototype.select = function select() {
  InterfaceTips.hideWallTip();

  if (this.selected) {
    UserInterface.reset();
    Overlays.hide();
  } else {
    UserInterface.select(this.localIndex);
    Overlays.show('BuildOverlay');
  }
  Sound.play('select');
};

WallFactory.prototype.action = function action() {
  InterfaceTips.hideWallTip();
  if (this.stacks_current === 0) {
    return Sound.play('error');
  }
  const cursor = Input.position();
  const tile = pointToTile(cursor.x, cursor.y);

  // Check if we're allowed to build here
  if (BuildOverlay.validPosition(tile[0], tile[1])) {
    // Reset UI if this is the last stack
    if (this.stacks_current === 1) {
      UserInterface.reset();
      Overlays.hide();
    }
    // Send output
    GameServer.emit('ActivateFactory', {
      index: 0,
      position: networkablePosition(tile),
    });
    // Add a wall drop
    Visuals.wallDrop(this.player_id);
  } else {
    UserInterface.reset();
  }
};

WallFactory.prototype.drawDragger = function drawDragger(context) {
  const cursor = Input.position();
  context.globalAlpha = 0.75;
  context.drawImage(
    Cache.get(`unit_1_${Game.playerIndex}`),
    cursor.x - 96,
    cursor.y - 96,
    192,
    192
  );
  context.globalAlpha = 1;
};
