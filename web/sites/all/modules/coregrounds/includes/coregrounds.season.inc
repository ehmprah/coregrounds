<?php

/**
 * Form constructor
 */
function coregrounds_finalize_season_form($form, &$form_state) {

  $form['current_key'] = array(
    '#markup' => 'Current Season: "' . variable_get('coregrounds_current_season', 'ALPHA') . '"'
  );

  $form['oldkey'] = array(
    '#type' => 'value',
    '#value' => variable_get('coregrounds_current_season', 'ALPHA'),
  );

  $form['key'] = array(
    '#type' => 'textfield',
    '#title' => 'New Season',
    '#maxlength' => 16,
    '#required' => TRUE,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Finalize Season'),
  );

  $form['#submit'][] = 'coregrounds_finalize_season_form_submit';

  return $form;
}

/**
 * Submit handler starting a migration batch
 */
function coregrounds_finalize_season_form_submit(&$form, &$form_state) {
  // Save new season key
  variable_set('coregrounds_current_season', $form_state['values']['key']);
  // Start the batch
  $batch = array(
    'operations' => array(
      array('coregrounds_finalize_season_batch', array($form_state['values']['oldkey'])),
    ),
    'title' => t('Wrapping up previous season'),
    'file' => drupal_get_path('module', 'coregrounds') . '/includes/coregrounds.season.inc',
  );
  batch_set($batch);
}

/**
 * Submit handler starting a migration batch
 */
function coregrounds_finalize_season_batch($oldkey, &$context) {

  $types = [
    'bot_wins',
    'bot_games',
    'bot_rank',
    'bot_winrate',
    'casual_wins',
    'casual_games',
    'casual_rank',
    'casual_winrate',
    'ranked_wins',
    'ranked_games',
    'ranked_rank',
    'ranked_winrate',
  ];

  if (!isset($context['sandbox']['total'])) {
    $query = "SELECT COUNT(*) FROM users WHERE uid > 0";
    $context['sandbox']['total'] = db_query($query)->fetchField();
    $context['sandbox']['processed'] = 0;
    $context['sandbox']['last'] = 0;
    // Load the current scoregrounds top 100 once
    $s = "SELECT uid FROM coregrounds_seasons
          WHERE season = :key AND ranked_rank > 0
          ORDER BY ranked_rank DESC LIMIT 100";
    $context['sandbox']['scoregrounds'] = db_query($s, [':key' => $oldkey])->fetchCol();
  }

  // We handle 5 users at a time
  $query = "SELECT * FROM users WHERE uid > :uid ORDER BY uid ASC LIMIT 5";
  foreach (db_query($query, array(':uid' => $context['sandbox']['last'])) as $account) {
    $total = [];

    // Load previous seasons
    $seasons = "SELECT * FROM coregrounds_seasons WHERE uid = :uid AND season != 'TOTAL'";
    foreach (db_query($seasons, [':uid' => $account->uid]) as $season) {
      foreach ($types as $type) {
        $total[$type][] = $season->{$type};
      }
    }

    // Calculate mean across all seasons
    foreach ($types as $type) {
      $fields[$type] = array_sum($total[$type]) / count($total[$type]);
    }

    // Insert mean as total season
    db_merge('coregrounds_seasons')->key([
      'season' => 'TOTAL',
      'uid' => $account->uid,
    ])->fields($fields)->execute();

    // Create new season entry
    db_insert('coregrounds_seasons')->fields([
      'uid' => $account->uid,
      'season' => variable_get('coregrounds_current_season', 'ALPHA'),
    ])->execute();

    // Unlock ranking achievements
    $ranked = array_search($account->uid, $context['sandbox']['scoregrounds']);
    if ($ranked !== FALSE) {
      $account = user_load($account->uid);
      if ($ranked == 0) coregrounds_update_achievements($account, 1, [204]);
      if ($ranked <= 9) coregrounds_update_achievements($account, 1, [203]);
      if ($ranked <= 24) coregrounds_update_achievements($account, 1, [202]);
      if ($ranked <= 49) coregrounds_update_achievements($account, 1, [201]);
      coregrounds_update_achievements($account, 1, [200]);
    }

    // We continue in the loop anyway so we don't get stuck
    $context['sandbox']['processed']++;
    $context['sandbox']['last'] = $account->uid;
  }

  // Keep the batch API posted.
  $context['finished'] = $context['sandbox']['processed'] / $context['sandbox']['total'];
}
