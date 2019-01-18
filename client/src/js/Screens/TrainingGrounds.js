/**
 * @file
 * Screen which shows
 */
import { todayString } from '../Util';
import Icons from '../Screens/Elements/Icons';
import { request } from '../API';

let videos = {};

Screens.add('traininggrounds', () => {
  const today = todayString();
  const raw = window.localStorage.getItem('cgs-traininggrounds');
  // If we have data, parse the json
  if (raw) videos = JSON.parse(raw);
  // If we do not have an up-to-date homepage, get it now
  if (videos.updated === today) {
    buildTrainingGrounds();
  } else {
    Screens.update('traininggrounds', `
      <throbber>LOADING TRAINING GROUNDS</throbber>
    `);
    request('app/training-grounds', false, (response) => {
      window.localStorage.setItem('cgs-traininggrounds', JSON.stringify({
        updated: today,
        list: response.data.videos,
      }));
      videos.list = response.data.videos;
      buildTrainingGrounds();
    });
  }
});

function buildTrainingGrounds() {
  let list = '';
  videos.list.forEach((id) => {
    list += `
      <li>
        <img onclick="Screens.show('video', 0, '${id}')" src="http://img.youtube.com/vi/${id}/maxresdefault.jpg">
      </li>
    `;
  });
  Screens.update('traininggrounds', `
    <div class="container">
      <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content">
        <ul class="content-row-3">
        ${list}
        </ul>
      </div>
    </div>
  `);
}
