/**
 * @file
 * Status pages
 */
import Analytics from '../Analytics';

Screens.add('status', (msg) => {
  Screens.update('status', `
    <throbber>${msg}</throbber>
  `);
});

Screens.add('maintenance', () => {
  Screens.update('maintenance', `
    <throbber>
      <h3 class="first text-310">MAINTENANCE</h3>
      <p>Coregrounds is currently under maintenance. We will be back shortly!</p>
      <div class="menu" style="margin: auto">
        <button class="box" onclick="window.open('https://www.reddit.com/r/coregrounds/comments/8a1d54/coregrounds_service_status');">Server Status</button>
        <button class="box grey" onclick="window.close()">QUIT</a>
      </div>
    </throbber>
  `);
});

Screens.add('auth-error', (msg) => {
  Analytics.exception(msg);
  Screens.update('auth-error', `
    <throbber class="error color-310">
      <h3 class="first text-310">AUTHENTICATION ERROR</h3>
      <p>Could not send authentication request.</p>
      <div class="menu" style="margin: auto">
        <button class="box grey" onclick="window.close()">Quit</a>
      </div>
    </throbber>
  `);
});

Screens.add('error', (msg) => {
  Analytics.exception(msg);
  Screens.update('error', `
    <throbber class="error color-310">
      <h3 class="first text-310">ERROR</h3>
      <p>${msg}</p>
      <div class="menu" style="margin: auto">
        <button class="box grey" onclick="Screens.show('home')">Back</a>
      </div>
    </throbber>
  `);
});

Screens.add('network-error', () => {
  Screens.update('network-error', `
    <throbber class="error color-310">
      <h3 class="first text-310">CONNECTION ERROR</h3>
      <p>Please check your internet connection.</p>
      <div class="menu" style="margin: auto">
        <button class="box" onclick="window.open('https://www.reddit.com/r/coregrounds/comments/8a1d54/coregrounds_service_status');">Server Status</button>
        <button class="box grey" onclick="Screens.show('home')">Back</button>
      </div>
    </throbber>
  `);
});

Screens.add('connection-lost', () => {
  Screens.update('connection-lost', `
    <throbber class="error color-310">
      <h3 class="first text-310">CONNECTION LOST</h3>
      <p>Please check your internet connection.</p>
      <div class="menu" style="margin: auto">
        <button class="box" onclick="Screens.show('play')">Reconnect</a>
      </div>
    </throbber>
  `);
});
