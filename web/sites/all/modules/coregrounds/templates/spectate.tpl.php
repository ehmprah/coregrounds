<a href="/" id="gohome" class="box padless">Quit</a>
<div class="client container">
  <object id="sprites" data="/sites/all/modules/coregrounds/client/img/sprites.svg?v=<?php print variable_get('coregrounds_version'); ?>" type="image/svg+xml" width="0" height="0"></object>

  <div id="msgs" class="vcenter clearfix">
    <canvas id="throbber"></canvas>
    <div id="connecting"><span class="msg">CONNECTING TO SERVER</span><a class="box" href="#" id="quit">Back</a></div>
  </div>

  <div id="menu" class="vcenter clearfix">
    <div id="games"></div>
    <a href="#" class="box" id="refresh">Refresh</a>
    <a class="box" href="/" id="quit">Back</a>
  </div>

  <div id="settings">
    <div class="overlay-box vcenter clearfix">
      <div class="volume-slider">
        <div class="clearfix">Sound Volume<span class="fr">100%</span></div>
        <input type="range" value="100" step="5" min="0" max="100" id="sound">
      </div>
      <div class="volume-slider">
        <div class="clearfix">Music Volume<span class="fr">100%</span></div>
        <input type="range" value="100" step="5" min="0" max="100" id="music">
      </div>
      <a class="cta centered" href="#" id="surrender"><span>Surrender</span></a>
      <a class="cta centered" href="#" id="resume"><span>Back</span></a>
    </div>
  </div>

  <div id="info">
    <div class="overlay-container vcenter clearfix">
    <div class="left-side">
      <div class="player-name"></div>
      <div class="factories"></div>
      <div class="abilities"></div>
    </div>
    <div class="right-side">
      <div class="player-name"></div>
      <div class="factories"></div>
      <div class="abilities"></div>
    </div>
    </div>
  </div>

  <div id="canvases" class="vcenter" style="display:none;" data-href="https://coregrounds.com/player/<?php print $account->uid; ?>/matches">
    <canvas id="map"></canvas>
    <canvas id="ui"></canvas>
  </div> 
</div>

<script src="https://coregrounds.com:1337/socket.io/socket.io.js"></script>
<script src="https://coregrounds.com/sites/all/modules/coregrounds/client/js/spectate.min.js?v=<?php print variable_get('coregrounds_version'); ?>"></script>