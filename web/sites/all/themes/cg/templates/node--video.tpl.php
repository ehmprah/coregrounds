<?php if($view_mode == 'featured' || $view_mode == 'teaser'): ?>
  <article class="l-2col">
    <a href="#" class="load-video" data-video="<?php print $node->field_youtube_id['und'][0]['value']; ?>">
      <img src="https://img.youtube.com/vi/<?php print $node->field_youtube_id['und'][0]['value']; ?>/maxresdefault.jpg">
      <svg viewBox="0 0 1024 1024">
        <path class="bg" d="M128 320l96-96c44.519 0 82.35 0 128 0l32 32h256l32-32h128l96 96v384l-96 96h-128l-32-32h-256l-32 32h-128l-96-96z"></path>
        <path class="play" d="M400 377.6l224 134.4-224 134.4z"></path>
      </svg>
    </a>
  </article>
<?php endif; ?>
<?php if($view_mode == 'full'): ?>
  <article class="video">
    <iframe width="1280" height="720" src="https://www.youtube-nocookie.com/embed/<?php print $node->field_youtube_id['und'][0]['value']; ?>?rel=0" frameborder="0" allowfullscreen></iframe>
  </article>
<?php endif; ?>