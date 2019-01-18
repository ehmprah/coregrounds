<div class="profile-data upper">
  <ul class="flex-row-3">
    <li class="blackbox pad">
      <div><?php print check_plain($account->name); ?></div>
      <div class="emblem"><?php print coregrounds_get_emblem($account); ?></div>
    </li>
    <li class="blackbox pad">
      <div>Rank</div>
      <div class="huge mv-15"><?php print 50-floor($account->season->ranked_rank / 5); ?></div>
      <div class="clearfix rank rank-<?php print $account->season->ranked_rank % 5; ?>">
        <div class="rb"></div>
        <div class="rb"></div>
        <div class="rb"></div>
        <div class="rb"></div>
        <div class="rb"></div>
      </div>
    </li>
    <li class="blackbox pad">
      <div>Win Streak</div>
      <div class="huge mv-15"><?php print $account->data->pvp_winstreak; ?></div>
      <div>Matches</div>
    </li>
  </ul>
  <?php foreach (['season' => 'Current Season', 'previous' => 'Previous Seasons'] as $key => $title): ?>
    <ul class="flex-row-3">
      <li class="blackbox pad">
        <div class="dark"><?php print $title; ?></div>
        <div>Bot Wins</div>
        <div class="huge mv-15"><?php print $account->{$key}->bot_wins; ?></div>
        <?php if($account->{$key}->bot_games): ?>
          <div>W/L: <?php print number_format($account->{$key}->bot_winrate * 100, 1); ?>%</div>
        <?php else: ?>
          <div>W/L: 0.0%</div>
        <?php endif; ?>
      </li>
      <li class="blackbox pad">
        <div class="dark"><?php print $title; ?></div>
        <div>Casual Wins</div>
        <div class="huge mv-15"><?php print $account->{$key}->casual_wins; ?></div>
        <?php if($account->{$key}->casual_games): ?>
          <div>W/L: <?php print number_format($account->{$key}->casual_winrate * 100, 1); ?>%</div>
        <?php else: ?>
          <div>W/L: 0.0%</div>
        <?php endif; ?>
      </li>
      <li class="blackbox pad">
        <div class="dark"><?php print $title; ?></div>
        <div>Ranked Wins</div>
        <div class="huge mv-15"><?php print $account->{$key}->ranked_wins; ?></div>
        <?php if($account->{$key}->ranked_games): ?>
          <div>W/L: <?php print number_format($account->{$key}->ranked_winrate * 100, 1); ?>%</div>
        <?php else: ?>
          <div>W/L: 0.0%</div>
        <?php endif; ?>
      </li>
    </ul>
  <?php endforeach; ?>

  <ul class="flex-row-4">
    <li class="blackbox pad">
      <div>Level</div>
      <div class="progress-bar mv-15"><div class="bar" style="width:<?php print $account->data->level_progress*100; ?>%"></div><span>Level <?php print $account->data->lvl; ?></span></div>
    </li>
    <li class="blackbox pad">
      <div>Credits earned</div>
      <div class="huge mv-15"><?php print $account->data->coins_earned; ?></div>
    </li>
    <li class="blackbox pad">
      <div>items unlocked</div>
      <div class="huge mv-15"><?php print count($account->unlocks)-4; ?></div>
    </li>
    <li class="blackbox pad">
      <div>Achievements</div>
      <div class="progress-bar mv-15"><div class="bar" style="width:<?php print $account->data->achievement_progress*100; ?>%"></div><span><?php print number_format(($account->data->achievement_progress*100),2); ?>%</span></div>
    </li>
  </ul>

</div>
