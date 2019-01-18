<div id="comments" class="<?php print $classes; ?>"<?php print $attributes; ?>>
  <?php if ($content['comments'] && $node->type != 'discussion'): ?>
    <h2 class="title"><?php print t('Comments'); ?></h2>
  <?php endif; ?>

  <?php print render($content['comments']); ?>

  <?php if ($content['comment_form']): ?>
    <h2 class="mv-15">Add comment</h2>
    <?php print render($content['comment_form']); ?>
  <?php endif; ?>
</div>
