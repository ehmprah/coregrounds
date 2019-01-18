<li class="blackbox achievement<?php print $progress >= $achievement['threshold'] ? ' achievement-unlocked' : ''; ?>">
  <div class="achievement-header">
    <div class="upper"><?php print $achievement['title']; ?></div>
    <div><?php print $achievement['credits']; ?> CREDITS</div>
  </div>
  <div class="progress-bar hlf mv-5">
    <div class="bar" style="width:<?php print ($progress / $achievement['threshold'] * 100); ?>%"></div>
    <span class="relative"><?php print number_format(($progress / $achievement['threshold'] * 100),2); ?>%</span>
  </div>
  <div class="achievement-description"><?php print $achievement['description']; ?></div>
</li>
