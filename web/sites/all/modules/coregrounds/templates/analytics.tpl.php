<div class="analytics">
  <ul class="flex-row-4 centered upper">
    <li class="blackbox pad" style="display:block;">
      <div>Daily New Users</div>
      <div class="huge mv-15"><?php print $analytics['users_new']; ?></div>
    </li>
    <li class="blackbox pad" style="display:block;">
      <div>Daily Active Users</div>
      <div class="huge mv-15"><?php print $analytics['dau']; ?></div>
    </li>
    <li class="blackbox pad" style="display:block;">
      <div>Monthly Active Users</div>
      <div class="huge mv-15"><?php print $analytics['mau']; ?></div>
    </li>
    <li class="blackbox pad" style="display:block;">
      <div>Total Users</div>
      <div class="huge mv-15"><?php print $analytics['users_total']; ?></div>
    </li>
  </ul>

  <ul class="flex-row-5 centered upper">
    <li class="blackbox pad" style="display:block;">
      <div>Ranked Bot</div>
      <div class="huge mv-15">
        <?= isset($analytics['matches'][0]) ? $analytics['matches'][0] : 0; ?>
      </div>
    </li>
    <li class="blackbox pad" style="display:block;">
      <div>Custom Bot</div>
      <div class="huge mv-15">
        <?= isset($analytics['matches'][5]) ? $analytics['matches'][5] : 0; ?>
      </div>
    </li>
    <li class="blackbox pad" style="display:block;">
      <div>Private</div>
      <div class="huge mv-15">
        <?= isset($analytics['matches'][3]) ? $analytics['matches'][3] : 0; ?>
      </div>
    </li>
    <li class="blackbox pad" style="display:block;">
      <div>Casual</div>
      <div class="huge mv-15">
        <?= isset($analytics['matches'][1]) ? $analytics['matches'][1] : 0; ?>
      </div>
    </li>
    <li class="blackbox pad" style="display:block;">
      <div>Ranked</div>
      <div class="huge mv-15">
        <?= isset($analytics['matches'][2]) ? $analytics['matches'][2] : 0; ?>
      </div>
    </li>
  </ul>

  <ul class="flex-row-2 centered upper">
    <li class="blackbox pad" style="display:block;">
      <div>Losses against Bot 1</div>
      <div class="huge mv-15"><?= number_format($analytics['first_bot_lost'], 1); ?>%</div>
    </li>
    <li class="blackbox pad" style="display:block;">
      <div>Surrenders against Bot 1</div>
      <div class="huge mv-15"><?= number_format($analytics['first_bot_surrender'], 1); ?>%</div>
    </li>
  </ul>

  <ul class="flex-row-5 centered upper">
    <?php foreach($analytics['bot_stats'] as $stat): ?>
      <li class="blackbox pad" style="display:block;">
        <div>Beat Bot <?= $stat['level']; ?></div>
        <div class="huge mv-15"><?= number_format($stat['ratio'], 1); ?>%</div>
      </li>
    <?php endforeach; ?>
  </ul>

  <ul class="flex-row-3 centered upper">
    <li class="blackbox pad" style="display:block;">
      <div>Retention D1</div>
      <div class="huge mv-15"><?php print number_format($analytics['d1'], 1); ?>%</div>
    </li>
    <li class="blackbox pad" style="display:block;">
      <div>Retention D7</div>
      <div class="huge mv-15"><?php print number_format($analytics['d7'], 1); ?>%</div>
    </li>
    <li class="blackbox pad" style="display:block;">
      <div>Retention D28</div>
      <div class="huge mv-15"><?php print number_format($analytics['d28'], 1); ?>%</div>
    </li>
  </ul>

  <ul class="flex-row-2 centered upper">
    <li class="blackbox pad" style="display:block;">
      <div>Subscriptions</div>
      <div class="huge mv-15"><?php print number_format($analytics['subscribers'], 2); ?>%</div>
    </li>
    <li class="blackbox pad" style="display:block;">
      <div>Conversions</div>
      <div class="huge mv-15"><?php print number_format($analytics['conversions'], 2); ?>%</div>
    </li>
  </ul>

</div>
