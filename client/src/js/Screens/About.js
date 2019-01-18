/**
 * @file
 * The about screen
 */
import Icons from '../Screens/Elements/Icons';

Screens.add('about', () => {
  Screens.update('about', `
    <div class="container-full">
      <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content">
        <div class="content-text">
          <h1>About Coregrounds</h1>
          <div style="float: left;">
            <img src="img/office.jpg" style="max-width: 400px;">
            <p style="color:#666; font-size: 16px;">ehmprah in his office</p>
          </div>
          <div style="float: right;">
            <img src="img/dnkpp.jpg" style="max-width: 400px;">
            <p style="color:#666; font-size: 16px;">DNKpp in his office</p>
          </div>
          <div>
            After working on the concept for the game for two years, Mario "ehmprah" Kaiser built a
            proof-of-concept version of Coregrounds for half a year and released it by the end of 2015.
            Together with a small community the core gameplay was evolved and battle-tested before Mario
            teamed up with Dominic "DNKpp" Koepke to level up the technology behind Coregrounds.
            The all-new version of Coregrounds is scheduled for a 2018 release on Steam, Google Play Store and Apple Store.
          </div>
          <h3>Coregrounds is built by these fine people:</h3>
          <ul class="list">
            <li>Mario "ehmprah" Kaiser, Creator & Code</li>
            <li>Dominic "DNKpp" Koepke, Code</li>
            <li>Simon "scukl" B&auml;se, Game Design</li>
            <li>John William Waters, Unit Art</li>
            <li>Michael Champion, Music</li>
            <li>LYREBIRD GAME + AUDIO GMBH, Sound Effects</li>
            <li>Mathias Tournier, Advisor</li>
          </ul>
          <h3>We thank these awesome humans</h3>
          <ul class="list">
            <li>Andreas</li>
            <li>Anna-Maria</li>
            <li>Annett</li>
            <li>Bernie</li>
            <li>c0nfuzed</li>
            <li>Edgar</li>
            <li>Flo</li>
            <li>H8</li>
            <li>Helmut</li>
            <li>Isabel</li>
            <li>Jakob</li>
            <li>Judith</li>
            <li>Knappe8</li>
            <li>Laura</li>
            <li>Marjan</li>
            <li>Mary</li>
            <li>Morgan</li>
            <li>Max</li>
            <li>Neza</li>
            <li>Oli</li>
            <li>Pascal</li>
            <li>Patrick</li>
            <li>Philomena</li>
            <li>Rainer</li>
            <li>Richard</li>
            <li>Robyn</li>
            <li>Rosa</li>
            <li>Rub&eacute;n</li>
            <li>Skadi</li>
            <li>Stephan</li>
            <li>Tanja</li>
            <li>Tobias</li>
          </ul>
          <h3>We use this awesome stuff</h3>
          <ul class="list">
            <li>Exo 2 by <a href="http://www.ndiscovered.com/">Natanael Gama</a></li>
            <li>Google's <a href="https://material.io/icons/">Material Icons</a></li>
            <li><a href="https://www.toptal.com/designers/subtlepatterns">Subtle Patterns</a></li>
          </ul>
          <h3>Follow us and join the conversation</h3>
          <nav class="follow">
            <a class="follow follow-discord" target="follow" title="Join us on Discord" href="http://discord.gg/xQmYUPr">Discord</a>
            <a class="follow follow-reddit" target="follow" title="Join us on Reddit" href="https://www.reddit.com/r/coregrounds">Reddit</a>
            <a class="follow follow-facebook" target="follow" title="Follow us on Facebook" href="https://www.facebook.com/coregrounds">Facebook</a>
            <a class="follow follow-twitter" target="follow" title="Follow us on Twitter" href="https://twitter.com/coregrounds">Twitter</a>
            <a class="follow follow-youtube" target="follow" title="Follow us on YouTube" href="https://www.youtube.com/coregrounds">YouTube</a>
            <a class="follow follow-indiedb" target="follow" title="Follow us on IndieDB" href="http://www.indiedb.com/games/coregrounds">IndieDB</a>
          </nav>
        </div>
      </div>
    </div>
  `);
});
