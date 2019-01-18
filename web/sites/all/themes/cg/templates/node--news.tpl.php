<?php if($view_mode == 'featured'): ?>
  <article class="news l-2col <?php print $view_mode; ?>">
    <a href="<?php print $node_url; ?>" rel="bookmark">
      <?php if(!isset($node->field_image['und'][0]['width']) || !isset($node->field_image['und'][0]['height']) || $node->field_image['und'][0]['width'] != 480 || $node->field_image['und'][0]['height'] != 270): ?>
        <?php print theme('image_style', array('style_name' => '480px', 'path' => $node->field_image['und'][0]['uri'])); ?>
      <?php else: ?>
        <img src="<?php print file_create_url($node->field_image['und'][0]['uri']); ?>" title="<?php print $title; ?>" alt="<?php print $title; ?>">
      <?php endif; ?>
      <div class="title-overlay">
        <h2 class="nm"><?php print $title; ?></h2>
      </div>
    </a>
  </article>
<?php endif; ?>

<?php if($view_mode == 'teaser'): ?>
  <article class="news box <?php print $view_mode; ?>">
    <?php if(isset($node->field_subtitle['und'][0]['value']) && !empty($node->field_subtitle['und'][0]['value'])): ?>
      <h6><?php print $node->field_subtitle['und'][0]['value']; ?></h6>
    <?php endif; ?>
    <h2><a href="<?php print $node_url; ?>" rel="bookmark"><?php print $title; ?></a></h2>
    <div class="grey small mb10">by <?php print $name; ?> about <?php print coregrounds_how_long_ago($node->created); ?></div>
    <?php hide($content['links']); ?>
    <?php hide($content['comments']); ?>

    <div class="content">
      <?php print render($content); ?>
    </div>

    <?php print render($content['comments']); ?>
  </article>
<?php endif; ?>

<?php if($view_mode == 'full'): ?>
  <article class="news <?php print $view_mode; ?>">
    <?php if(isset($node->field_subtitle['und'][0]['value']) && !empty($node->field_subtitle['und'][0]['value'])): ?>
      <h6><?php print $node->field_subtitle['und'][0]['value']; ?></h6>
    <?php endif; ?>
    <h1><?php print $title; ?></h1>
    <div class="grey small mb10">by <?php print $name; ?> about <?php print coregrounds_how_long_ago($node->created); ?></div>
    <?php hide($content['links']); ?>
    <?php hide($content['comments']); ?>
    <div class="content">
      <?php print render($content); ?>
    </div>
    <div class="share">
      <?php $reddit_url = 'https://www.reddit.com/r/coregrounds/'; ?>
      <?php if (isset($node->field_reddit_url['und'][0]['value']) && !empty($node->field_reddit_url['und'][0]['value'])): ?>
        <?php $reddit_url = $node->field_reddit_url['und'][0]['value']; ?>
      <?php endif; ?>
      <a class="reddit" onclick="gtag('event', 'discuss', { 'event_label': 'Reddit' });" target="_blank" href="<?php print $reddit_url; ?>">
        <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 341.8 341.8"><path d="M336.83 173.525c0-20.122-16.33-36.453-36.452-36.453-9.915 0-18.663 3.791-25.079 10.207-24.787-17.789-59.198-29.453-97.109-30.912l16.623-77.862 53.949 11.374c.583 13.706 11.956 24.787 25.954 24.787 14.29 0 25.954-11.665 25.954-25.954 0-14.29-11.665-25.954-25.954-25.954-10.207 0-18.955 5.832-23.038 14.581l-60.365-12.831c-1.75-.292-3.5 0-4.957.875-1.458.875-2.333 2.333-2.916 4.082l-18.372 86.902c-38.786 1.167-73.488 12.54-98.567 30.912-6.416-6.124-15.456-10.207-25.08-10.207-20.121 0-36.452 16.33-36.452 36.453 0 14.872 8.749 27.412 21.58 33.244-.583 3.5-.875 7.29-.875 11.081 0 55.991 65.031 101.192 145.518 101.192 80.486 0 145.517-45.2 145.517-101.192 0-3.79-.292-7.29-.875-10.79 11.956-5.832 20.997-18.663 20.997-33.535zM87.498 199.479c0-14.29 11.665-25.954 25.954-25.954 14.29 0 25.954 11.664 25.954 25.954 0 14.289-11.664 25.954-25.954 25.954-14.289 0-25.954-11.665-25.954-25.954zm144.934 68.53c-17.788 17.788-51.616 18.955-61.531 18.955s-44.034-1.458-61.531-18.955c-2.625-2.625-2.625-7 0-9.624 2.624-2.624 6.998-2.624 9.623 0 11.081 11.082 34.994 15.165 52.2 15.165 17.205 0 40.826-4.083 52.2-15.165 2.624-2.624 6.998-2.624 9.623 0 2.04 2.917 2.04 7-.584 9.624zm-4.666-42.576c-14.289 0-25.954-11.665-25.954-25.954 0-14.29 11.665-25.954 25.954-25.954 14.29 0 25.954 11.664 25.954 25.954 0 14.289-11.664 25.954-25.954 25.954z" fill="#fff" stroke-width="2.916"/></svg>
        <span>Discuss</span>
      </a>
      <a class="facebook" onclick="gtag('event', 'share', { 'event_label': 'Facebook' });" target="_blank" href="http://www.facebook.com/sharer.php?u=https://coregrounds.com<?php print $node_url; ?>">
        <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" width="24" height="24"><path d="M18.768 7.465H14.5V5.56c0-.896.594-1.105 1.012-1.105H18.5V.513L14.171.5C10.244.5 9.5 3.438 9.5 5.32v2.144h-3v4h3v12h5v-12h3.851l.417-3.999z"/><path fill="none" d="M0 0h24v24H0z"/></svg>
        <span>Share</span>
      </a>
      <a class="twitter" onclick="gtag('event', 'share', { 'event_label': 'Twitter' });" target="_blank" href="http://twitter.com/share?url=https://coregrounds.com<?php print $node_url; ?>">
        <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" width="24" height="24"><path d="M23.444 4.834c-.814.363-1.5.375-2.227.016.938-.562.981-.957 1.32-2.019-.878.521-1.851.9-2.886 1.104a4.545 4.545 0 0 0-7.743 4.145 12.897 12.897 0 0 1-9.366-4.748 4.525 4.525 0 0 0-.615 2.285c0 1.577.803 2.967 2.021 3.782a4.527 4.527 0 0 1-2.057-.568l-.001.057a4.547 4.547 0 0 0 3.646 4.456 4.575 4.575 0 0 1-2.053.079 4.551 4.551 0 0 0 4.245 3.155A9.138 9.138 0 0 1 1 18.459 12.865 12.865 0 0 0 7.966 20.5c8.358 0 12.928-6.924  12.928-12.929 0-.198-.003-.393-.012-.588.887-.64 1.953-1.237 2.562-2.149z"/><path fill="none" d="M0 0h24v24H0z"/></svg>
        <span>Tweet</span>
      </a>
      <a class="mail" onclick="gtag('event', 'share', { 'event_label': 'Mail' });" target="_blank" href="mailto:?body=https://coregrounds.com<?php print $node_url; ?>">
        <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" width="24" height="24"><path d="M12.002 12.36l10.095-8.03A1.99 1.99 0 0 0 21.001 4h-18c-.387 0-.746.115-1.053.307l10.054 8.053z"/><path d="M22.764 5.076l-10.468 8.315a.488.488 0 0 1-.594-.001L1.26 5.036c-.16.287-.259.612-.26.964v11c.001 1.103.898 2 2.001 2h17.998c1.103 0 2-.897 2.001-2V6c0-.335-.09-.647-.236-.924z"/><g><path fill="none" d="M0 0h24v24H0z"/></g></svg>
        <span>Email</span>
      </a>
    </div>


  </article>
<?php endif; ?>
