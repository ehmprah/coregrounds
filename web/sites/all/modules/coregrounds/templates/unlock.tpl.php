<?php if($type == 'towers' || $type == 'minions' || $type == 'abilities'): ?>
  <div class="box padded l-2col relative unlock-<?php print $type; ?>" id="unlock-<?php print $item['unlid']; ?>">
<?php else: ?>
  <div class="box padded l-3col relative unlock-<?php print $type; ?>" id="unlock-<?php print $item['unlid']; ?>">
<?php endif; ?>

  <?php if($type == 'towers' || $type == 'minions' || $type == 'abilities'): ?>
    <div class="unlock-sprite"><?php print coregrounds_build_sprite($item['unlid'],$GLOBALS['user']->data->color,128); ?></div>
    <div class="unlock-title"><?php print $item['name']; ?></div>
    <div class="unlock-desc"><?php print $item['desc']; ?></div>
  <?php endif; ?>

  <?php if($type == 'modifications'): ?>
    <div class="unlock-sprite"><div class="icon-amp"></div></div>
    <div class="unlock-title"><?php print $item['group']; ?></div>
    <div class="unlock-desc"><?php print $item['title']; ?></div>
  <?php endif; ?>

  <?php if($type == 'emotes'): ?>
    <div class="unlock-sprite"><div class="icon-emote"></div></div>
    <div class="unlock-title">Emote</div>
    <div class="unlock-desc">"<?php print $item['msg']; ?>"</div>
  <?php endif; ?>

  <?php if($type == 'emblems'): ?>
    <div class="unlock-sprite"><img src="<?php print $item['url']; ?>"></div>
    <div class="unlock-title"><?php print $item['name']; ?></div>
    <div class="unlock-desc"></div>
  <?php endif; ?>

  <?php if($type == 'colors'): ?>
    <div class="clearfix">
      <div class="unlock-sprite"><?php print coregrounds_build_sprite(4,$item['unlid'],128); ?></div>
      <div class="unlock-title"><?php print $item['title']; ?></div>
    </div>
  <?php endif; ?>

  <?php if(in_array($item['unlid'],coregrounds_get_free_dailies())): ?>
    <div class="ribbon"><span>free</span></div>
  <?php endif; ?>

  <?php if(!in_array($item['unlid'], $account->unlocks)): ?>
    <a class="unlock-btn use-ajax" href="/unlock/item/<?php print $item['unlid']; ?>/nojs">
      <span class="price"><span class="credit-icon"></span><span><?php print $item['price']; ?></span></span>
      <span class="cta">Unlock</span>
    </a>
  <?php else: ?>
    <div class="unlocked"><span>Unlocked<a class="fr" href="/player/<?php print $GLOBALS['user']->uid; ?>/refunds">Refund</a></span></div>
  <?php endif; ?>

</div>