/**
 * @file
 * Provides a rectangular clickable area on the screen
 */
import Input from '../../Input';

function Box(x, y, width, height) {
  this.x = x;
  this.y = y;

  this.width = width;
  this.height = height;

  this.hover = false;
  this.active = false;

  // Add a listening flag to turn input boxes on and off easily
  this.listening = true;

  // Default events for every input box
  this.on('mousein', this.hoverOn.bind(this));
  this.on('mouseout', this.hoverOff.bind(this));
  this.on('mousedown', this.activeOn.bind(this));

  // We don't have to bind the mouseup event to this box
  document.addEventListener('mouseup', this.activeOff.bind(this));

  // Dragstart callback
  this.dragStart = null;
}

Box.prototype.activeY = function activeY(y) {
  if (y) return this.active ? y + 2 : y;
  return this.active ? this.y + 2 : this.y;
};

Box.prototype.on = function on(event, fn) {
  // Catch any additional args given
  const args = Array.prototype.splice.call(arguments, 2);
  // Add the normal params
  args.unshift(fn);
  args.unshift(event);
  args.unshift(this);
  // Apply args to Input's on function
  Input.on.apply(null, args);
};

Box.prototype.off = function off(event, fn) {
  Input.off(this, event, fn);
};

Box.prototype.hoverOn = function hoverOn() {
  this.hover = true;
};

Box.prototype.hoverOff = function hoverOff() {
  this.hover = false;
  // Also toggle the active flag
  if (this.active) {
    if (this.dragStart) this.dragStart();
    this.active = false;
  }
};

Box.prototype.activeOn = function activeOn() {
  this.active = true;
};

Box.prototype.activeOff = function activeOff() {
  this.active = false;
};

Box.prototype.activeOffset = function activeOffset(y) {
  if (this.active) y += 2;
  return y;
};

export default Box;
