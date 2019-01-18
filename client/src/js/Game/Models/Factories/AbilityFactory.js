/**
 * AbilityFactory.js
 * Holds the networked game data about the factory and handles user interaction
 */
import { color } from '../../../Graphics/Color';
import { networkablePosition } from '../../../Util/Geometry';
import Box from '../Box';
import Config from '../../../Config/Config';
import Cooldown from '../../../Graphics/Elements/Cooldown';
import Input from '../../../Input';
import GameServer from '../../../Network/GameServer';
import Sound from '../../../Sound';
import UnitCache from '../../../Graphics/UnitCache';
import UserInterface from '../../../Interface/UserInterface';

export default function AbilityFactory(eid, index, playerIndex) {
  this.eid = eid;
  this.cooldown_percentage = 0;
  this.cooldown_seconds = 0;
  this.effects = [];

  this.index = index;
  this.selected = false;
  this.player_id = playerIndex;
  this.box = new Box(32 + (index * 128), 924, 128, 128, true);
  this.box.dragStart = this.dragStart.bind(this);
  this.box.on('mouseup', this.select.bind(this));

  const item = Config.query('abilities', { id: this.eid });
  this.name = item.name.toUpperCase();
}

AbilityFactory.prototype.draw = function draw(context, tx, ty) {
  const x = tx || this.box.x;
  let y = ty || this.box.y;
  if (this.box.active && this.cooldown_seconds === 0) {
    y += 2;
  }

  // Route interface icon through the cooldown handler
  Cooldown.draw(
    context,
    x,
    y,
    this.eid,
    this.player_id,
    'Ability',
    'interface',
    this.cooldown_percentage,
    this.cooldown_seconds
  );

  // Draw status effects
  if (this.effects.length) {
    context.statusFX(x, y, this.effects);
  }
};

AbilityFactory.prototype.select = function select() {
  Sound.play('select');
  if (this.selected) {
    UserInterface.reset();
  } else {
    UserInterface.select(this.index);
    UserInterface.dragStart(this.index);
    return false;
  }
};

AbilityFactory.prototype.dragStart = function dragStart() {
  UserInterface.select(this.index);
  UserInterface.dragStart(this.index);
};

AbilityFactory.prototype.dragEnd = function dragEnd() {
  if (this.cooldown_seconds === 0) {
    this.action();
  } else {
    Sound.play('error');
  }
  UserInterface.reset();
};

AbilityFactory.prototype.action = function action() {
  if (this.cooldown_seconds === 0) {
    GameServer.emit('ActivateAbility', {
      index: this.index - 6,
      position: networkablePosition(),
    });
  }
  UserInterface.reset();
};

AbilityFactory.prototype.drawDragger = function drawDragger(context) {
  // TODO add tile based targeting dragger for ionstrike and co
  // Exception for mass production
  if (this.eid === 1410) {
    // Just display a green hue as opposed to the red one over the UI area
    context.fillStyle = 'rgba(0, 150, 0, 0.1)';
    context.fillRect(0, 0, 1920, 896);
    return;
  }

  const cursor = Input.position();

  let range = 0;

  // Ability: Adrenaline
  if (this.eid === 1411) {
    const spell = Config.query('spells', { id: 14114499 });
    range = spell.area.circle.radius * 128;
  }

  // Ability: Lifelink
  if (this.eid === 1413) {
    const spell = Config.query('spells', { id: 14134499 });
    range = spell.area.circle.radius * 128;
  }

  // Ability: Heal
  if (this.eid === 1400) {
    const spell = Config.query('spells', { id: 14004499 });
    range = spell.area.circle.radius * 128;
  }

  // Ability: Explode
  if (this.eid === 1401) {
    const spell = Config.query('spells', { id: 14014499 });
    range = spell.area.circle.radius * 128;
  }

  // Ability: Slow
  if (this.eid === 1405) {
    const spell = Config.query('spells', { id: 14054499 });
    range = spell.area.circle.radius * 128;
  }

  // Ability: Hurry
  if (this.eid === 1406) {
    const spell = Config.query('spells', { id: 14064499 });
    range = spell.area.circle.radius * 128;
  }

  // Ability: Frenzy
  if (this.eid === 1407) {
    const spell = Config.query('spells', { id: 14074499 });
    range = spell.area.circle.radius * 128;
  }

  // Ability: Shield
  if (this.eid === 1408) {
    const spell = Config.query('spells', { id: 14084499 });
    range = spell.area.circle.radius * 128;
  }

  // Ability: Stun
  if (this.eid === 1409) {
    const spell = Config.query('spells', { id: 14094499 });
    range = spell.area.circle.radius * 128;
  }

  // Ability: Exhaust
  if (this.eid === 1415) {
    const spell = Config.query('spells', { id: 14154499 });
    range = spell.area.circle.radius * 128;
  }

  // Ability: Root
  if (this.eid === 1417) {
    const spell = Config.query('spells', { id: 14174499 });
    range = spell.area.circle.radius * 128;
  }

  // If this ability has a range, display a range indicator
  if (range) {
    context.fillStyle = color(0.2);
    context.beginPath();
    context.arc(cursor.x, cursor.y, range, 0, 2 * Math.PI);
    context.closePath();
    context.fill();
  }

  // Draw ability icon
  context.drawImage(
    UnitCache.get(this.eid, this.player_id, 'Ability'),
    cursor.x - 64,
    cursor.y - 64,
    128,
    128
  );
};
