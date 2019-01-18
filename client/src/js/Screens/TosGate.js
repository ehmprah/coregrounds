/**
 * @file
 * Adds a gate for updated terms of service
 */
import { request } from '../API';
import { finalize } from '../Session/Session';

Screens.add('tosgate', (tos) => {
  Screens.update('tosgate', `
    <div class="container">
      <div class="tos">
        <div class="tos-content">${tos}</div>
        <div class="tos-accept">
          <div class="buttons">
            <button class="box grey" id="deny-tos">CANCEL</button>
            <button class="box" id="accept-tos">ACCEPT</button>
          </div>
        </div>
      </div>
    </div>
  `);

  $('#accept-tos').addEventListener('click', () => {
    // Send request to webserver
    request('app/agree-tos', false, () => {
      Screens.hide(1);
      finalize();
    });
  });

  $('#deny-tos').addEventListener('click', () => {
    window.close();
  });
});
