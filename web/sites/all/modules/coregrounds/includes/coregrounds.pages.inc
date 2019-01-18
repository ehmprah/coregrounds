<?php

/**
 * Implementation of hook_cron().
 */
function coregrounds_front() {

  drupal_add_html_head(array(
    '#tag' => 'meta',
    '#attributes' => array(
      'property' => 'og:image',
      'content' => 'https://coregrounds.com/sites/default/files/og_image.png',
    ),
  ), 'og_image');

  drupal_add_html_head(array(
    '#tag' => 'meta',
    '#attributes' => array(
      'name' => 'description',
      'content' => 'Tower Defense meets MOBA in this fast-paced, competive free-to-play strategy game with cross-platform multiplayer.',
    ),
  ), 'meta_description');

  return array();
}

/**
 * Implementation of hook_cron().
 */
function coregrounds_match($mid) {
  if($match = coregrounds_load_match(check_plain($mid))) {
    $page[] = array(
      '#theme' => 'match',
      '#match' => $match,
      '#view_mode' => 'full',
    );
    return $page;
  }
  return MENU_NOT_FOUND;
}

/**
 * Altered version of taxonomy_term_page().
 */
function coregrounds_term_page($term) {
  // If there is a menu link to this term, the link becomes the last part of
  // the active trail, and the link name becomes the page title. Thus, we must
  // explicitly set the page title to be the term title.
  drupal_set_title($term->name);

  // Set the term path as the canonical URL to prevent duplicate content.
  $uri = entity_uri('taxonomy_term', $term);
  drupal_add_html_head_link(array('rel' => 'canonical', 'href' => url($uri['path'], $uri['options'])), TRUE);
  // Set the non-aliased path as a default shortlink.
  drupal_add_html_head_link(array('rel' => 'shortlink', 'href' => url($uri['path'], array_merge($uri['options'], array('alias' => TRUE)))), TRUE);

  if($term->vid == 5) {
    $build['tabs'] = array(
      '#markup' => '<div class="clearfix centered upper"><a class="l-3col box padless" href="/game-guide/towers">Towers</a><a class="l-3col box padless" href="/game-guide/minions">Minions</a><a class="l-3col box padless" href="/game-guide/abilities">Abilities</a></div>',
    );
    if($nids = taxonomy_select_nodes($term->tid, TRUE, 50)) {
      $nodes = node_load_multiple($nids);
      $build['nodes']['#prefix'] = '<div class="clearfix">';
      $build['nodes']['#suffix'] = '</div>';
      $build['nodes']['content'] = node_view_multiple($nodes);
      $build['pager'] = array(
        '#theme' => 'pager',
        '#weight' => 5,
      );
    } else {
      $build['no_content'] = array(
        '#markup' => '<p>No content has been created here yet.</p>',
      );
    }
    return $build;
  }

  $build = array();
  if($nids = taxonomy_select_nodes($term->tid, TRUE, 10)) {
    $nodes = node_load_multiple($nids);
    $build += node_view_multiple($nodes);
    $build['pager'] = array(
      '#theme' => 'pager',
      '#weight' => 5,
    );
  } else {
    $build['no_content'] = array(
      '#markup' => '<p>No content has been created here yet.</p>',
    );
  }
  return $build;
}

/**
 * News page.
 */
function coregrounds_news() {

  $page['news'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('news')),
    'nodes' => array(),
  );

  //build page
  $query = db_select('node','n')
    ->fields('n',array('nid'))
    ->condition('n.type','news')
    ->condition('n.status', 1)
    ->orderBy('n.created','DESC')
    ->extend('PagerDefault')->limit(10);
  if($nids = $query->execute()->fetchCol()) {
    foreach(node_load_multiple($nids) as $node) {
      $page['news']['nodes'][] = node_view($node,'teaser');
    }
  }

  $page['pager'] = array('#theme' => 'pager');

  return $page;
}

/**
 * Implementation of hook_cron().
 */
function coregrounds_stats($type) {
  return array('#markup' => 'Under construction!');
  // $cfg = coregrounds_get_settings();

  drupal_set_title('Statistics '.($type == 'pickrate' ? ' - Pick Rate' : ' - Win Rate'));

  foreach($cfg['factories'] as $name => $item) {
    if(isset($item['unlid'])) {
      $factories[$item['unlid']]['sprite'] = coregrounds_build_sprite($item['unlid'],$GLOBALS['user']->data->color,48);
      $factories[$item['unlid']]['name'] = $name;
      $factories[$item['unlid']]['picks'] = 0;
      $factories[$item['unlid']]['wins'] = 0;
    }
  }
  foreach($cfg['abilities'] as $name => $item) {
    $abilities[$item['unlid']]['sprite'] = coregrounds_build_sprite($item['unlid'],$GLOBALS['user']->data->color,48);
    $abilities[$item['unlid']]['name'] = $name;
    $abilities[$item['unlid']]['picks'] = 0;
    $abilities[$item['unlid']]['wins'] = 0;
  }

  $games = 0;
  //load stats
  foreach (db_query('SELECT * FROM {coregrounds_statistics} WHERE season = :season AND patch = :patch', array(
    ':season' => variable_get('coregrounds_current_season'),
    ':patch' => variable_get('coregrounds_version','0.0.0'),
    )) as $pick) {
    if(isset($factories[$pick->unlid])) {
      $factories[$pick->unlid]['picks'] = $pick->picks;
      $factories[$pick->unlid]['wins'] = $pick->wins;
      $games += $pick->picks;
    }
    if(isset($abilities[$pick->unlid])) {
      $abilities[$pick->unlid]['picks'] = $pick->picks;
      $abilities[$pick->unlid]['wins'] = $pick->wins;
      $games += $pick->picks;
    }
  }

  foreach($factories as $unlid => $factory) {
    $picks = $factories[$unlid]['picks'];
    $wins = $factories[$unlid]['wins'];
    $sort_factories[$unlid] = $type == 'pickrate' ? ($picks / ($games/5) * 100) : ($wins / $picks * 100);
    $factories[$unlid]['picks'] = number_format(($picks / ($games/5) * 100),1).' %';
    $factories[$unlid]['wins'] = $picks ? number_format(($wins / $picks * 100),1).' %' : '0%';
  }

  foreach($abilities as $unlid => $ability) {
    $picks = $abilities[$unlid]['picks'];
    $wins = $abilities[$unlid]['wins'];
    $sort_abilities[$unlid] = $type == 'pickrate' ? ($picks / ($games/5) * 100) : ($wins / $picks * 100);
    $abilities[$unlid]['picks'] = number_format(($picks / ($games/5) * 100),1).' %';
    $abilities[$unlid]['wins'] = $picks ? number_format(($wins / $picks * 100),1).' %' : '0%';
  }

  arsort($sort_factories);
  $rows_factories = array();
  foreach($sort_factories as $unlid => $val){
    $rows_factories[$unlid] = $factories[$unlid];
  }

  arsort($sort_abilities);
  $rows_abilities = array();
  foreach($sort_abilities as $unlid => $val){
    $rows_abilities[$unlid] = $abilities[$unlid];
  }

  return array(
    'factories' => array(
      '#theme' => 'table',
      '#header' => array('Unit','','Picks','Wins'),
      '#rows' => $rows_factories,
      '#sticky' => 0,
      '#prefix' => '<div class="l-2col stats"><h2>Units</h2>',
      '#suffix' => '</div>',
    ),
    'abilities' => array(
      '#theme' => 'table',
      '#header' => array('Ability','','Picks','Wins'),
      '#rows' => $rows_abilities,
      '#sticky' => 0,
      '#prefix' => '<div class="l-2col stats"><h2>Abilities</h2>',
      '#suffix' => '</div>',
    ),
  );
}

/**
 * Implementation of hook_cron().
 */
function coregrounds_leaderboard($type) { //ADD EMBLEM!

  drupal_set_title('Scoregrounds - '.ucfirst($type));

  $page['title'] = array('#markup' => '<h1>'.'Scoregrounds - '.ucfirst($type).'</h1>');

  //add tables to query
  $query = db_select('users','u');
  $query->join('coregrounds_players','p','p.uid = u.uid');
  $query->join('coregrounds_seasons','s','s.uid = u.uid AND season = :season',array(':season' => variable_get('coregrounds_current_season')));
  //add fields to query
  $query->fields('u', array('name','uid'));
  $query->fields('p', array('coins_earned','achievement_progress'));
  $query->fields('s',array('casual_wins','casual_games','ranked_wins','ranked_games','ranked_rank'));
  //limit
  $query->range(0,100); //maybe add paging?

  //ordering
  if($type == 'casual') {
    $query->orderBy('s.casual_rank','DESC');
    $query->orderBy('s.casual_winrate','DESC');
    $query->orderBy('s.casual_wins','DESC');
  }
  if($type == 'ranked') {
    $query->orderBy('s.ranked_rank','DESC');
    $query->orderBy('s.ranked_winrate','DESC');
    $query->orderBy('s.ranked_wins','DESC');
  }
  if($type == 'achievements') {
    $query->orderBy('p.achievement_progress','DESC');
  }
  if($type == 'coins') {
    $query->orderBy('p.coins_earned','DESC');
  }

  $rows = array();
  $rank = 0;
  foreach($query->execute() as $player) {
    $row = array();
    $rank++;
    $row[] = $rank.'.';
    $row[] = l(check_plain($player->name),'user/'.$player->uid);
    $row[] = $player->casual_wins.' / '.($player->casual_games-$player->casual_wins);
    $row[] = $player->ranked_wins.' / '.($player->ranked_games-$player->ranked_wins);
    $row[] = 50-floor($player->ranked_rank / 5);
    $row[] = $player->coins_earned;
    $row[] = '<div class="progress-bar"><div class="bar" style="width:'.floor($player->achievement_progress*100).'%"></div><span class="relative">'.number_format($player->achievement_progress*100,1).'</span></div>';
    $rows[] = $row;
  }

  $page['leaderboards'] = array(
    '#theme' => 'table',
    '#header' => array('','Player','Casual','Ranked','Rank','Coins','Achievements'),
    '#rows' => $rows,
    '#sticky' => 0,
    '#attributes' => array('id' => 'leaderboards'),
  );

  return $page;
}
