<?php if($view_mode == 'teaser'): ?>

  <article class="box boxed-teaser voting">
    <div class="comment-count">
      <?php print $comment_count; ?><br><span class="grey small">Comments</span>
      <?php if(isset($content['links']['comment']['#links']['comment-new-comments']['title'])): ?>
        <div><a class="small" href="<?php print $node_url; ?>#new"><?php print str_replace(array(' comments',' comment'),'',$content['links']['comment']['#links']['comment-new-comments']['title']); ?></a></div>
      <?php endif; ?>
    </div>
    <div class="content">
      <div><a href="<?php print $node_url; ?>" rel="bookmark" class="upper"><?php print $title; ?></a></div>
      <div class="grey small">
        <?php print coregrounds_how_long_ago($node->created); ?> by <?php print $name; ?> in <a href="/devgrounds?tid=<?php print $node->field_idea_for['und'][0]['tid']; ?>"><?php print render($content['field_idea_for']); ?></a>
      </div>
      <div class=""><?php print render($content['field_description']); ?></div>
    </div>
    <?php if(isset($node->field_todo['und'][0]['value']) && $node->field_todo['und'][0]['value']): ?>
      <div class="ribbon"><span>CONFIRMED</span></div>
    <?php endif; ?>
  </article>

<?php else: ?>

  <article class="box">

    <div class="small"><a href="/devgrounds">Devgrounds</a> &raquo; <a href="/devgrounds?tid=<?php print $node->field_idea_for['und'][0]['tid']; ?>"><?php print render($content['field_idea_for']); ?></a></div>
    <h1 class="nm"><?php print $title; ?></h1>
    <div class="grey small mb10"><?php print coregrounds_how_long_ago($node->created); ?> by <?php print $name; ?></div>

    <?php hide($content['links']); ?>
    <?php hide($content['comments']); ?>

    <div<?php print $content_attributes; ?>>
      <?php print render($content); ?>
    </div>

  </article>

    <?php //ADD MINI SHARE BUTTONS HERE AND AT COMMENTS! ?>

    <?php print render($content['comments']); ?>



<?php endif; ?>
