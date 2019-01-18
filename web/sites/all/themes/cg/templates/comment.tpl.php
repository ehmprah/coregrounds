<div class="box <?php print $classes; ?>"<?php print $attributes; ?>>
  <div class="grey small clearfix mb10"><?php print coregrounds_how_long_ago($comment->created); ?> by <?php print $author; ?><span class="fr"><?php print render($content['links']) ?></span></div>
  <div class="content"<?php print $content_attributes; ?>>
    <?php print render($content); ?>
  </div>
</div>