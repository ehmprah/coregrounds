/**
 * @file
 * Screen which shows the most recent news
 *
 * TODO add news image
 * TODO add link to news page on coregrounds.com
 */
import Icons from '../Screens/Elements/Icons';

Screens.add('news', () => {
  const news = JSON.parse(window.localStorage.getItem('cgs-home')).data.news[0];
  Screens.update('news', `
    <div class="container">
      <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content">
        <h1>${news.title}</h1>
        ${news.subtitle ? `<h3>${news.subtitle}</h3>` : ''}
        ${news.html}
      </div>
    </div>
  `);
});
