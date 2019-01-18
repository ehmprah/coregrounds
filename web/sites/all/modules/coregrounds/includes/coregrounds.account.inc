<?php

/**
 * Our custom profile page
 */
function coregrounds_account($account) {
  if(!$account->uid) {
    return MENU_NOT_FOUND;
  }
  //set title
  drupal_set_title(check_plain($account->name));
  //return themed output
  return array(
    '#theme' => 'profile',
    '#account' => $account,
  );
}

/**
 * Our custom profile page
 */
function coregrounds_achievements($user) {
  if(!$user->uid) {
    return MENU_NOT_FOUND;
  }
  //set title
  drupal_set_title(check_plain($user->name).'\'s Achievements');

  //get this users achievemnts
  coregrounds_get_achievements($user);

  $completed = isset($_GET['show']) ? TRUE : FALSE;

  // Display overall progress
  $page['total'] = array(
    '#markup' => '
    <div class="blackbox mv">
      <h6 class="centered">Achievements</h6>
      <div class="progress-bar mv">
        <div class="bar" style="width:'.($user->data->achievement_progress*100).'%"></div>
        <span>'.number_format(($user->data->achievement_progress*100),2).'%</span>
      </div>
      <div class="mv centered">
        <a href="/player/'.$user->uid.'/achievements'.($completed ? '' : '?show=completed').'">
        '.($completed ? 'Show incomplete' : 'Show completed').'
        </a>
      </div>
    </div>',
  );

  $page['achievements'] = array(
    '#prefix' => '<ul class="achievements flex-row-2">',
    '#suffix' => '</ul>',
  );

  foreach (configQuery('achievements') as $achievement) {
    $progress = isset($user->achievements[$achievement['id']])
      ? $user->achievements[$achievement['id']]->progress
      : 0;
    if ($completed || $progress < $achievement['threshold']) {
      $page['achievements'][] = array(
        '#theme' => 'achievement',
        '#achievement' => $achievement,
        '#progress' => $progress,
      );
    }
  }

  return $page;
}

/**
 * Our custom profile page
 */
function coregrounds_matches($account) {
  if (!$account->uid) {
    return MENU_NOT_FOUND;
  }

  drupal_set_title(check_plain($account->name).'\'s Match History');
  $matches = array();

  // Load 10 matches
  $query = db_select('coregrounds_matches','m')->orderBy('m.ended','DESC')->extend('pagerDefault')->limit(10);
  $query->join('coregrounds_matches_players', 'p', 'p.mid = m.mid AND p.uid = :uid',
    array(':uid' => $account->uid));
  $query->join('coregrounds_matches_players', 'p2', 'p2.mid = m.mid AND p2.uid != :uid',
    array(':uid' => $account->uid));
  $query->fields('m', array('mid','match_type','won','duration','ended'));
  $query->fields('p', array('team', 'coins', 'xp'));
  $query->fields('p2', array('name'));

  foreach($query->execute() as $match) {
    $match->color = $account->data->color;
    $matches[] = array(
      '#theme' => 'match',
      '#match' => $match,
      '#view_mode' => 'match_history',
    );
  }

  if (!$matches) {
    return array('#markup' => '<p>This player hasn\'t played any matches yet.</p>');
  }

  return [
    'matches' => [
      '#prefix' => '<ul class="matches">',
      '#suffix' => '</ul>',
      'list' => $matches,
    ],
    'pager' => ['#theme' => 'pager']
  ];
}

/**
 * Our custom profile page
 */
function coregrounds_modify_page_form($form,&$form_state,$pid = 0) {
  global $user;

  //add libraries for ajax handling
  drupal_add_library('system', 'drupal.ajax');
  drupal_add_library('system', 'jquery.form');

  //load settings, user and pages
  // $cfg = coregrounds_get_settings();
  $account = user_load($user->uid);
  $pages = coregrounds_get_modification_pages($account->uid); //dvp(coregrounds_get_modification_pages($account->uid,1));
  $pid = check_plain($pid);

  if(!$pid) {
    $pid = $account->data->last_used_page;
  }
  if(!$pid) {
    $pid = key($pages);
  }

  //build option array
  $options['offensive'] = array();
  $options['defensive'] = array();
  $options['utility'] = array();
  // foreach($cfg['modifications'] as $item) {
  //   if(in_array($item['unlid'],$account->unlocks)) {
  //     $options[$item['group']][$item['unlid']] = $item['title'];
  //   }
  // }

  $form['#prefix'] = '<h1 class="mb">Your modification pages</h1>';
  $form['#attributes']['class'][] = 'clearfix';
  $form['#attributes']['class'][] = 'styled';
  $form['pages'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('clearfix')),
    'page' => array(
      '#type' => 'container',
      '#attributes' => array('class' => array('upgrade-page','box','l-3q')),
    ),
    'select' => array(
      '#type' => 'container',
      '#attributes' => array('class' => array('l-1q')),
    ),
  );

  foreach($pages as $page) {
    if($page['pid'] == $pid) {

      $form['pages']['select'][] = array(
        '#markup' => '<div class="box select-page selected">'.(isset($page['title']) ? $page['title'] : 'Untitled page').'</div>',
      );

      $form['pages']['page']['pid'] = array(
        '#type' => 'value',
        '#value' => $pid,
      );

      $form['pages']['page']['save']['#tree'] = 1;

      $form['pages']['page']['save']['title'] = array(
        '#type' => 'textfield',
        '#title' => 'Title',
        '#default_value' => (isset($page['title']) ? $page['title'] : 'Untitled page'),
      );

      foreach(array('offensive','defensive','utility') as $group) {
        foreach(array(1,2,3,4,5) as $i) {
          $form['pages']['page']['save']['mods'][$group][$i] = array(
            '#type' => 'select',
            '#title' => strtoupper($group),
            '#options' => $options[$group],
            '#empty_option' => 'Choose a modification',
            '#default_value' => (isset($page['mods'][$group][$i]) ? $page['mods'][$group][$i] : 0),
          );
        }
      }

      $form['pages']['page']['actions'] = array(
        '#type' => 'container',
        '#attributes' => array('class' => array('actions')),
        'save' => array(
          '#type' => 'submit',
          '#value' => 'Save',
        ),
      );

    } else {
      $form['pages']['select'][] = array(
        '#markup' => '<a class="select-page box" href="/modify/'.$page['pid'].'">'.(isset($page['title']) ? $page['title'] : 'Untitled page').'</a>',
      );
    }
  }

  //add unlock page button!
  $form['pages']['select'][] = array(
    '#markup' => '<div class="select-page mv"><a class="unlock-btn" href="/unlock/page"><span class="price"><span class="credit-icon"></span><span>'.$cfg['prices']['upgrade_page'].'</span></span><span class="cta">Add page</span></a></div>',
  );

  return $form;
}

/**
 * Submit handler for the upgrade page form
 */
function coregrounds_modify_page_form_submit($form,&$form_state) {
  db_update('coregrounds_modifications')->fields(array(
    "title" => $form_state['values']['save']['title'],
    "mods" => json_encode(clean_array($form_state['values']['save']['mods'])),
  ))->condition('pid',$form_state['values']['pid'])->execute();
  drupal_set_message('Your modification page was saved.');
}

/**
 * Add a page function
 */
function coregrounds_unlock_page() {
  global $user;
  $account = user_load($user->uid);
  $cfg = coregrounds_get_settings();
  if($account->data->coins >= $cfg['prices']['upgrade_page']) {
    if(lock_acquire('user_unlock_'.$user->uid)) {
      db_update('coregrounds_players')->fields(array(
        'coins' => ($account->data->coins - $cfg['prices']['upgrade_page']),
        'modification_pages' => $account->data->modification_pages + 1,
      ))->condition('uid',$account->uid)->execute();
      db_insert('coregrounds_modifications')->fields(array('uid' => $account->uid,'title' => 'Untitled page'))->execute();
      lock_release('user_unlock_'.$user->uid);
      drupal_goto('modify');
    }
  } else {
    drupal_set_message('You do not have enough credits for this item.','error');
    drupal_goto('modify');
  }
}
