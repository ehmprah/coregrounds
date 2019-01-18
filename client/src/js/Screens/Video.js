/**
 * @file
 * Screen which plays YouTube videos
 */
import Config from '../Config/Config';
import Event from '../Event';
import Icons from '../Screens/Elements/Icons';

let volume;

Event.on('showing video', () => {
  volume = Config.User.get('music-volume', 1);
  Config.User.set('music-volume', 0);
  Event.fire('music volume changed');
});

Event.on('leaving video', () => {
  Config.User.set('music-volume', volume);
  Event.fire('music volume changed');
});

Screens.add('video', (id) => {
  Screens.update('video', `
    <div class="container">
      <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content">
        <div class="iframe">
          <iframe allowfullscreen src="https://www.youtube.com/embed/${id}?autoplay=1&disablekb=1&&wmode=opaque&enablejsapi=1" style="position:absolute;top:0;left:0;width:100%;height:100%;border:0;"></iframe>
        </div>
      </div>
    </div>
  `);
});
