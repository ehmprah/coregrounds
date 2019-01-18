/**
 * @file
 * Handles overlay selection widget
 */
let callback;

Screens.add('select-overlay', (items, cb) => {
  // Update callback
  callback = cb;
  // Build HTML for the selectable items
  let options = '';
  items.forEach((item) => {
    options += `<button class="box" onclick="overlaySelect(${item.id})">${item.title}</button>`;
  });
  if (!options) {
    options = 'none available';
  }
  // Update screen with select options
  Screens.update('select-overlay', `
    <div class="container-full bg-contrast flex" onclick="Screens.hide(1)">
      <div class="select-overlay">
        ${options}
      </div>
    </div>
  `);
});

window.overlaySelect = function overlaySelect(id) {
  callback(id);
};
