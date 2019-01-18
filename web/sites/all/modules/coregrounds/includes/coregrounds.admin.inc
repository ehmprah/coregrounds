<?php
/**
 * Settings form
 */
function coregrounds_settings_form($form,&$form_state) {

  $form['coregrounds_tos_version'] = array(
    '#type' => 'textfield',
    '#title' => 'TOS Version Number',
    '#default_value' => variable_get('coregrounds_tos_version', '0.0.0'),
  );

  $form['coregrounds_app_maintenance'] = array(
    '#type' => 'checkbox',
    '#title' => 'App Maintenance',
    '#default_value' => variable_get('coregrounds_app_maintenance', 0),
  );

  $form['coregrounds_stream_live'] = array(
    '#type' => 'checkbox',
    '#title' => 'Twitch Stream live',
    '#default_value' => variable_get('coregrounds_stream_live',0),
  );

  $form['coregrounds_stream_msg'] = array(
    '#type' => 'textarea',
    '#title' => 'Stream Message',
    '#rows' => 3,
    '#default_value' => variable_get('coregrounds_stream_msg',""),
  );

  $form['coregrounds_global_bonus'] = array(
    '#type' => 'textarea',
    '#title' => 'Global Bonus',
    '#rows' => 3,
    '#default_value' => variable_get('coregrounds_global_bonus',''),
    '#description' => 'Format YYYY-MM-DD',
  );

  $form['coregrounds_global_bonus_ranked'] = array(
    '#type' => 'textarea',
    '#title' => 'Global Bonus Ranked',
    '#rows' => 3,
    '#default_value' => variable_get('coregrounds_global_bonus_ranked',''),
    '#description' => 'Format YYYY-MM-DD',
  );

  $form['actions'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('clearfix')),
  );

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Save',
    '#submit' => array('coregrounds_settings_form_submit'),
  );

  $form['secondary'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('clearfix','mv')),
  );

  return $form;
}

/**
 * Settings form submit handler
 */
function coregrounds_settings_form_submit($form,&$form_state) {
  foreach($form_state['values'] as $key => $value) {
    variable_set($key,$value);
  }
  drupal_set_message('Settings saved.');
}

/**
 * Settings form
 */
function coregrounds_newsletter_subscribers() {
  $query = db_select('users','u')->fields('u',array('name','mail','created'))->condition('u.status',1);
  $query->join('field_data_field_newsletter','f','f.entity_id = u.uid AND field_newsletter_value = 1');
  foreach($query->execute() as $user) {
    $recipients[] = array(check_plain($user->name),$user->mail);
  }
  //create file
  $fp = fopen('Coregrounds_subscribers.csv', 'w');
  //insert values into file
  foreach ($recipients as $recipient) {
    fputcsv($fp, $recipient);
  }
  //close file handle
  fclose($fp);
  //send file to browser
  header('Content-type: text/csv');
  header("Content-Disposition: attachment; filename=\"Coregrounds_subscribers.csv\"");
  readfile('Coregrounds_subscribers.csv');
  //delete the file and exit
  unlink('Coregrounds_subscribers.csv');
  drupal_exit();
}

/**
 * Analytics - All matches
 */
function coregrounds_all_matches() {
  $page = array();

  $query = db_select('coregrounds_matches','m')->orderBy('m.ended','DESC')->extend('pagerDefault')->limit(10);
  $query->join('coregrounds_matches_players','p1','p1.mid = m.mid AND p1.player = 0');
  $query->join('coregrounds_matches_players','p2','p2.mid = m.mid AND p2.player = 1');
  $query->fields('m',array('mid','match_type','won','duration','ended'));
  $query->fields('p1',array('player','name'));
  $query->fields('p2',array('player','name'));

  foreach($query->execute() as $match) {
    $page['matches'][] = array(
      '#markup' => '<a href="/match/'.$match->mid.'">'.$match->name . ($match->won == $match->player ? ' (won)' : '') .' vs '.$match->p2_name. ($match->won == $match->p2_player ? ' (won)' : '') . '</a><span class="fr">'.coregrounds_how_long_ago($match->ended).'</span>',
      '#prefix' => '<div class="box clearfix">',
      '#suffix' => '</div>',
    );
  }

  return $page;
}

/**
 * Helper function returning percentages
 */
function coregrounds_calc_diff($new,$old,$up = TRUE) {
  $diff = $new - $old;
  if($old) {
    $percentage = $diff / $old * 100;
    return  '<span class="'.(($up && $percentage > 0 || !$up && $percentage < 0) ? 'green' : 'red').'">'.sprintf("%+.2f",$percentage).'%</span>';
  }
  return '&ndash;';
}

/**
 * Analytics - Totals
 */
function coregrounds_totals() {

  $registered_kickstarter = db_query("SELECT COUNT(*) FROM users WHERE created > :ts", array(':ts' => 1467331200))->fetchField();
  drupal_set_message($registered_kickstarter . " new players since start of the Kickstarter.");
  $privates = db_query("SELECT COUNT(*) FROM coregrounds_matches WHERE match_type = 3 AND ended > :ts", array(':ts' => 1467133403))->fetchField();
  drupal_set_message($privates . " private matches since start of the Kickstarter.");
  $matches = db_query("SELECT COUNT(*) FROM coregrounds_matches WHERE ended > :ts", array(':ts' => 1467133403))->fetchField();
  drupal_set_message($matches . " matches since start of the Kickstarter.");
  $page['analytics'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('analytics')),
  );

  $page['analytics']['users'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('clearfix')),
    'online' => array(
      '#type' => 'container',
      '#attributes' => array('class' => array('l-2col','box')),
      'headline' => array('#markup' => '<div class="grey centered">LAST ONLINE</div>'),
    ),
    'usercount' => array(
      '#type' => 'container',
      '#attributes' => array('class' => array('l-4col','box')),
      'headline' => array('#markup' => '<div class="grey">TOTAL USERS</div>'),
      'content' => array(
        '#markup' => '<div class="huge mv">'.db_query("SELECT COUNT(*) FROM users")->fetchField().'</div>',
      ),
    ),
  );

  //recently online
  $query = db_select('users','u')->fields('u',array('uid','name','access'))->orderBy('u.access','DESC')->range(0,5);
  foreach($query->execute() as $result) {
    $page['analytics']['users']['online'][] = array(
      '#markup' => '<div class="clearfix"><a href="'.url('user/'.$result->uid).'">'.$result->name.'</a><span class="fr grey">'.coregrounds_how_long_ago($result->access).'</span></div>',
    );
  }

  //matches
  $matches = db_query("SELECT match_type, count(match_type) FROM coregrounds_matches GROUP BY match_type")->fetchAllKeyed();
  $page['analytics']['matches'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('clearfix','centered')),
    'bot' => array(
      '#type' => 'container',
      '#attributes' => array('class' => array('l-4col','box')),
      'headline' => array('#markup' => '<div class="grey">AI</div>'),
      'count' => array('#markup' => '<div class="huge mv">'.$matches[0].'</div>'),
    ),
    'casual' => array(
      '#type' => 'container',
      '#attributes' => array('class' => array('l-4col','box')),
      'headline' => array('#markup' => '<div class="grey">CASUAL</div>'),
      'count' => array('#markup' => '<div class="huge mv">'.$matches[1].'</div>'),
    ),
    'ranked' => array(
      '#type' => 'container',
      '#attributes' => array('class' => array('l-4col','box')),
      'headline' => array('#markup' => '<div class="grey">RANKED</div>'),
      'count' => array('#markup' => '<div class="huge mv">'.$matches[2].'</div>'),
    ),
    'private' => array(
      '#type' => 'container',
      '#attributes' => array('class' => array('l-4col','box')),
      'headline' => array('#markup' => '<div class="grey">PRIVATE</div>'),
      'count' => array('#markup' => '<div class="huge mv">'.$matches[3].'</div>'),
    ),
  );

  return $page;
}

/**
 * Settings form
 */
function coregrounds_color_checker_form($form,&$form_state) {

  if(isset($form_state['values']['colors'])) {
    $form['#suffix'] = '';
    //build sprites for all colors
    $cfg = coregrounds_get_settings();
    foreach($cfg['colors'] as $unlid => $stuff) {
      $form['#suffix'] .= coregrounds_build_sprite(4,$unlid,128);
    }
    foreach(explode("\n",$form_state['values']['colors']) as $hex) {
      $form['#suffix'] .= $hex . coregrounds_color_checker_svg_generator($hex);
    }
  }

  $form['colors'] = array(
    '#type' => 'textarea',
    '#title' => 'New Colors To Check',
    '#rows' => 5,
    '#default_value' => isset($form_state['values']['colors']) ? $form_state['values']['colors'] : '',
  );

  $form['actions'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('clearfix')),
  );

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Check colors',
  );

  return $form;
}

/**
 * Settings form submit handler
 */
function coregrounds_color_checker_form_submit($form,&$form_state) {
  $form_state['rebuild'] = 1;
}

function coregrounds_color_checker_svg_generator($hex) {
  list($r, $g, $b) = sscanf($hex, "#%02x%02x%02x");
  $svg = '<svg class="sprite" width="128" height="128" title="TANK">';
  $svg .= '<use class="base" style="fill: #222" xmlns:xlink="http://www.w3.org/1999/xlink" xlink:href="/sites/all/modules/coregrounds/client/img/sprites.svg?v='.variable_get('coregrounds_version').'#icon-minion"></use>';
  $svg .= '<use class="base" style="fill: rgba('.$r.','.$g.','.$b.',0.1)" xmlns:xlink="http://www.w3.org/1999/xlink" xlink:href="/sites/all/modules/coregrounds/client/img/sprites.svg?v='.variable_get('coregrounds_version').'#icon-minion"></use>';
  $svg .= '<use class="reset" xmlns:xlink="http://www.w3.org/1999/xlink" xlink:href="/sites/all/modules/coregrounds/client/img/sprites.svg?v='.variable_get('coregrounds_version').'#icon-minion"></use>';
  $svg .= '<use class="stroke" style="stroke: rgba('.$r.','.$g.','.$b.',0.3)" xmlns:xlink="http://www.w3.org/1999/xlink" xlink:href="/sites/all/modules/coregrounds/client/img/sprites.svg?v='.variable_get('coregrounds_version').'#icon-minion"></use>';
  $svg .= '<use class="icon" style="fill: rgba('.$r.','.$g.','.$b.',1)" xmlns:xlink="http://www.w3.org/1999/xlink" xlink:href="/sites/all/modules/coregrounds/client/img/sprites.svg?v='.variable_get('coregrounds_version').'#icon-tank"></use>';
  $svg .= '</svg>';
  return $svg;
}
