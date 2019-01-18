<?php if($view_mode == 'full'): ?>

<?php $abbreviations = configQuery('abbreviations'); ?>

<div class="mh-match">
  <div class="centered">
    <h2>Match Summary</h2>
    <p class="grey">Mode:
    <?php if($match->match_type == 0): ?>Bot<?php endif; ?>
    <?php if($match->match_type == 1): ?>Casual<?php endif; ?>
    <?php if($match->match_type == 2): ?>Ranked<?php endif; ?>
    <?php if($match->match_type == 3): ?>Private<?php endif; ?>
    <?php if($match->match_type == 4): ?>Random<?php endif; ?>
    <?php if($match->match_type == 5): ?>Custom Bot<?php endif; ?>
+++ Duration: <?php print gmdate("H:i:s", $match->duration); ?> +++ Date: <?php print date("F, d",$match->ended); ?></p>
  </div>

  <ul class="flex-row-2">
    <?php foreach($match->players as $player): ?>
      <li class="blackbox flex-column relative pad">
        <?php print ($match->won == $player->team) ? '<div class="ribbon"><span>WON</span></div>' : ''; ?>
        <div class="mh-player flex">
          <div class="mh-name">
            <?php if ($player->uid): ?>
              <a href="/player/<?php print $player->uid; ?>" class="text-<?php print $player->color; ?>">
                <?php print $player->name; ?>
              </a>
            <?php else: ?>
              <span><?php print $player->name; ?></span>
            <?php endif; ?>
          </div>
          <div class="currency">
            <svg xmlns="http://www.w3.org/2000/svg" width="128" height="128" viewBox="0 0 33.867 33.867"><path d="M16.933 1.565A15.368 15.368 0 0 0 1.565 16.933a15.368 15.368 0 0 0 15.368 15.368 15.368 15.368 0 0 0 15.368-15.368A15.368 15.368 0 0 0 16.933 1.565zm1.442 5.244c2.277 0 4.178.248 5.709.738a6.575 18.57 0 0 0-.472 2.31 57.737 57.737 0 0 0-4.682-.194c-2.324 0-3.936.515-4.834 1.545-.898 1.03-1.347 2.932-1.347 5.705 0 2.8.449 4.716 1.347 5.746.898 1.03 2.509 1.545 4.834 1.545 1.574 0 3.022-.073 4.347-.22a6.575 18.57 0 0 0 .358 2.472c-1.584.399-3.349.601-5.299.601-3.328 0-5.693-.753-7.093-2.258-1.373-1.532-2.06-4.161-2.06-7.886 0-3.725.687-6.34 2.06-7.845 1.4-1.506 3.778-2.258 7.132-2.258v-.001zm1.595 4.109c1.356 0 2.491.145 3.408.432a6.575 18.57 0 0 0-.175 1.41 34.696 34.696 0 0 0-2.9-.123c-1.4 0-2.37.31-2.911.93-.541.62-.81 1.766-.81 3.437 0 1.686.27 2.84.81 3.461.542.62 1.512.93 2.912.93.988 0 1.893-.049 2.718-.145a6.575 18.57 0 0 0 .12 1.5c-.955.241-2.02.363-3.196.363-2.005 0-3.429-.453-4.272-1.36-.827-.923-1.24-2.506-1.24-4.75 0-2.243.413-3.818 1.24-4.725.843-.906 2.276-1.36 4.296-1.36z"></path></svg>
            <span><?php print $player->coins; ?></span>
          </div>
          <div class="mh-xp">
            <?php print $player->xp; ?> XP
          </div>
        </div>
        <div class="mh-picks flex">
          <?php print coregrounds_build_sprite($player->ban_1, '#444'); ?>
          <?php print coregrounds_build_sprite($player->ban_2, '#444'); ?>
          <?php foreach($player->factories as $factory): ?>
            <?php print coregrounds_build_sprite($factory->id, coregrounds_get_color($player->color)); ?>
          <?php endforeach; ?>
          <?php foreach($player->abilities as $ability): ?>
            <?php print coregrounds_build_sprite($ability->id, coregrounds_get_color($player->color)); ?>
          <?php endforeach; ?>
        </div>
      </li>
    <?php endforeach; ?>
  </ul>

  <ul class="flex-row-5">
    <li class="blackbox match-stat">
      <div>SPAWNS</div>
      <div><span class="text-<?php print $match->players[0]->color; ?>"><?php print $match->players[0]->minions_spawned; ?></span> / <span class="text-<?php print $match->players[1]->color; ?>"><?php print $match->players[1]->minions_spawned; ?></span></div>
    </li>
    <li class="blackbox match-stat">
      <div>WALLS BUILT</div>
      <div><span class="text-<?php print $match->players[0]->color; ?>"><?php print $match->players[0]->walls_built; ?></span> / <span class="text-<?php print $match->players[1]->color; ?>"><?php print $match->players[1]->walls_built; ?></span></div>
    </li>
    <li class="blackbox match-stat">
      <div>WALL KILLS</div>
      <div><span class="text-<?php print $match->players[0]->color; ?>"><?php print $match->players[0]->walls_killed; ?></span> / <span class="text-<?php print $match->players[1]->color; ?>"><?php print $match->players[1]->walls_killed; ?></span></div>
    </li>
    <li class="blackbox match-stat">
      <div>TOWER KILLS</div>
      <div><span class="text-<?php print $match->players[0]->color; ?>"><?php print $match->players[0]->towers_killed; ?></span> / <span class="text-<?php print $match->players[1]->color; ?>"><?php print $match->players[1]->towers_killed; ?></span></div>
    </li>
    <li class="blackbox match-stat">
      <div>ABILITY USES</div>
      <div><span class="text-<?php print $match->players[0]->color; ?>"><?php print $match->players[0]->ability_uses; ?></span> / <span class="text-<?php print $match->players[1]->color; ?>"><?php print $match->players[1]->ability_uses; ?></span></div>
    </li>
    <li class="blackbox match-stat">
      <div>UPGRADES</div>
      <div><span class="text-<?php print $match->players[0]->color; ?>"><?php print $match->players[0]->upgrade_total; ?></span> / <span class="text-<?php print $match->players[1]->color; ?>"><?php print $match->players[1]->upgrade_total; ?></span></div>
    </li>
    <li class="blackbox match-stat">
      <div>XP GAINED</div>
      <div><span class="text-<?php print $match->players[0]->color; ?>"><?php print $match->players[0]->xp_total; ?></span> / <span class="text-<?php print $match->players[1]->color; ?>"><?php print $match->players[1]->xp_total; ?></span></div>
    </li>
    <li class="blackbox match-stat">
      <div>DAMAGE</div>
      <div><span class="text-<?php print $match->players[0]->color; ?>"><?php print number_format($match->players[0]->damage/1000,1); ?>k</span> / <span class="text-<?php print $match->players[1]->color; ?>"><?php print number_format($match->players[1]->damage/1000,1); ?>k</span></div>
    </li>
    <li class="blackbox match-stat">
      <div>HEALING</div>
      <div><span class="text-<?php print $match->players[0]->color; ?>"><?php print number_format($match->players[0]->healing/1000,1); ?>k</span> / <span class="text-<?php print $match->players[1]->color; ?>"><?php print number_format($match->players[1]->healing/1000,1); ?>k</span></div>
    </li>
    <li class="blackbox match-stat">
      <div>CORE</div>
      <div><span class="text-<?php print $match->players[0]->color; ?>"><?php print $match->players[0]->core_health; ?>%</span> / <span class="text-<?php print $match->players[1]->color; ?>"><?php print $match->players[1]->core_health; ?>%</span></div>
    </li>
  </ul>

  <ul class="flex-row-2">
    <?php foreach($match->players as $player): ?>
      <?php foreach($player->factories as $factory): ?>
        <li class="blackbox factory-info" style="order: <?= $factory->pickorder; ?>">
          <?php $item = configQuery('factories', 'id', $factory->id); ?>
          <?php $upgrades = isset($item['tower']) ? $item['tower']['upgrades'] : $item['minion']['upgrades']; ?>
          <?php print coregrounds_build_sprite($factory->id, coregrounds_get_color($player->color)); ?>
          <div class="upgrade">
            <div class="label"><?= $abbreviations[$upgrades[0]['name']]; ?></div>
            <div class="clearfix rank rank-<?= 1 + $factory->upgrade_1; ?>">
              <div></div><div></div><div></div><div></div><div></div>
            </div>
          </div>
          <div class="upgrade">
            <div class="label"><?= $abbreviations[$upgrades[1]['name']]; ?></div>
            <div class="clearfix rank rank-<?= 1 + $factory->upgrade_2; ?>">
              <div></div><div></div><div></div><div></div><div></div>
            </div>
          </div>
          <div class="upgrade">
            <div class="label"><?= $abbreviations[$upgrades[2]['name']]; ?></div>
            <div class="clearfix rank rank-<?= 1 + $factory->upgrade_3; ?>">
              <div></div><div></div><div></div><div></div><div></div>
            </div>
          </div>
          <svg style="margin: auto;" width="50" height="50" viewbox="0 0 100 100" version="1.1" xmlns="http://www.w3.org/2000/svg">
            <circle r="40" cx="50" cy="50" fill="transparent" stroke-dasharray="251.32" stroke-dashoffset="0" stroke-width="8" stroke="#000"></circle>
            <circle r="40" cx="50" cy="50" fill="transparent" stroke-dasharray="251.32"
              stroke-dashoffset="<?= $factory->xp > 3000 ? 0 : 252 - ((($factory->xp % 150) / 150) * pi() * 80); ?>" stroke-width="8" stroke="<?= coregrounds_get_color($player->color); ?>" style="transform: rotate(-90deg); transform-origin: 50%;"></circle>
            <text x="50%" y="50%" dy=".4em" fill="<?= coregrounds_get_color($player->color); ?>" style="text-anchor: middle; font-size: 30px;">
              <?= $factory->xp > 3000 ? 20 : floor($factory->xp / 150); ?>
            </text>
           </svg>
        </li>
      <?php endforeach; ?>
    <?php endforeach; ?>
  </ul>
</div>
<?php endif; ?>

<?php if($view_mode == 'match_history'): ?>
  <li>
    <a href="/match/<?php print $match->mid; ?>" class="block blackbox mb flex pad">
      <?php if ($match->won == $match->team): ?>
        <div class="won">WON</div>
      <?php else: ?>
        <div class="lost">LOST</div>
      <?php endif; ?>
      <div class="upper"><?= coregrounds_match_type_string($match->match_type); ?></div>
      <div><?php print gmdate("H:i:s", $match->duration); ?></div>
      <div class="currency">
        <svg xmlns="http://www.w3.org/2000/svg" width="128" height="128" viewBox="0 0 33.867 33.867"><path d="M16.933 1.565A15.368 15.368 0 0 0 1.565 16.933a15.368 15.368 0 0 0 15.368 15.368 15.368 15.368 0 0 0 15.368-15.368A15.368 15.368 0 0 0 16.933 1.565zm1.442 5.244c2.277 0 4.178.248 5.709.738a6.575 18.57 0 0 0-.472 2.31 57.737 57.737 0 0 0-4.682-.194c-2.324 0-3.936.515-4.834 1.545-.898 1.03-1.347 2.932-1.347 5.705 0 2.8.449 4.716 1.347 5.746.898 1.03 2.509 1.545 4.834 1.545 1.574 0 3.022-.073 4.347-.22a6.575 18.57 0 0 0 .358 2.472c-1.584.399-3.349.601-5.299.601-3.328 0-5.693-.753-7.093-2.258-1.373-1.532-2.06-4.161-2.06-7.886 0-3.725.687-6.34 2.06-7.845 1.4-1.506 3.778-2.258 7.132-2.258v-.001zm1.595 4.109c1.356 0 2.491.145 3.408.432a6.575 18.57 0 0 0-.175 1.41 34.696 34.696 0 0 0-2.9-.123c-1.4 0-2.37.31-2.911.93-.541.62-.81 1.766-.81 3.437 0 1.686.27 2.84.81 3.461.542.62 1.512.93 2.912.93.988 0 1.893-.049 2.718-.145a6.575 18.57 0 0 0 .12 1.5c-.955.241-2.02.363-3.196.363-2.005 0-3.429-.453-4.272-1.36-.827-.923-1.24-2.506-1.24-4.75 0-2.243.413-3.818 1.24-4.725.843-.906 2.276-1.36 4.296-1.36z"></path></svg>
        <span><?php print $match->coins; ?></span>
      </div>
      <div><?php print $match->xp; ?> XP</div>
      <div><?php print check_plain($match->name); ?></div>
    </a>
  </li>
<?php endif; ?>
