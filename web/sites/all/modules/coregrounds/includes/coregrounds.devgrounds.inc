<?php

/**
 * Implementation of hook_cron().
 */
function coregrounds_devgrounds() {

  $page['devgrounds'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('devgrounds','clearfix')),
    'header' => array(
      '#markup' => '<h1>Welcome to the Devgrounds</h1><p>Coregrounds has improved so much with your ideas and feedback! This is the new community hub for your ideas - you can post your ideas and discuss and vote on ideas by your fellow players. <span class="notes">All ideas that make it into the game will be rewarded. For example: if your unit makes it into the game, it will be unlocked for you for free!</span></p>',
    ),
    'filters' => array(
      '#type' => 'container',
      '#attributes' => array('class' => array('hlf','clearfix')),
    ),
    'content' => array(),
  );

  if(!user_is_logged_in()) {
    $page['devgrounds']['header']['#suffix'] = '<p><a href="/player" class="cta">Log in to post your idea</a></p>';
  } else {
    if(user_access('create discussion content')) {
      $page['devgrounds']['header']['#suffix'] = '<p><a href="/create/idea" class="cta">Add your idea</a></p>';
    } else {
      $page['devgrounds']['header']['#suffix'] = '<p>You have to <a href="/player/'.$GLOBALS['user']->uid.'/edit">validate your email adress</a> before you can post ideas.</p>';
    }
  }

  //get categories
  $tags = taxonomy_get_tree(6);

  //we start the query
  $query = db_select('node','n')->fields('n',array('nid'))->condition('n.type','idea')->extend('PagerDefault')->limit(25);

  $t = 0;
  if(isset($_GET['t'])) {
    $t = check_plain($_GET['t']);
  } else {
    $query->join('field_data_field_idea_for','f','f.entity_id = n.nid');
    $query->condition('f.field_idea_for_tid',21,'!=');
  }

  //add tag filtering selection
  $page['devgrounds']['filters'][0] = array('#markup' => '<a class="box padless l-6col active" href="/devgrounds">ALL</a>');
  foreach($tags as $term) {
    if($term->tid == $t) {
      $page['devgrounds']['filters'][] = array('#markup' => '<a class="box padless l-6col active" href="/devgrounds?t='.$term->tid.'">'.$term->name.'</a>');
      $page['devgrounds']['filters'][0] = array('#markup' => '<a class="box padless l-6col" href="/devgrounds">ALL</a>');
      $query->join('field_data_field_idea_for','f','f.entity_id = n.nid AND f.field_idea_for_tid = :tid', array(':tid' => $term->tid));
    } else {
      $page['devgrounds']['filters'][] = array('#markup' => '<a class="box padless l-6col" href="/devgrounds?t='.$term->tid.'">'.$term->name.'</a>');
    }
  }

  //order by votes
  // $query->leftjoin('votingapi_cache','v','v.entity_type = :type AND v.function = :func AND v.entity_id = n.nid',array(':type' => 'node',':func' => 'sum'));
  // $query->orderBy('v.value','DESC');
  if($nids = $query->execute()->fetchCol()) {
    foreach(node_load_multiple($nids) as $node) {
      $page['devgrounds']['content'][] = node_view($node,'teaser');
    }
  } else {
    $page['devgrounds']['content'][] = array('#markup' => '<div class="mv">No ideas have been posted here so far. Want to <a href="/create/idea">add yours?</a></div>');
  }

  $page['devgrounds']['pager'] = array('#theme' => 'pager');

  return $page;
}
