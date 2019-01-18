<?php

/**
 * @file
 * Loads players
 */

/**
 * Load a player
 */
function coregrounds_api_load_player() {
  if (isset($_POST['key']) && $_POST['key'] === COREGROUNDS_APP_KEY) {
    $sid = '';
    if (isset($_POST['sid'])) {
      $sid = $_POST['sid'];
    }

    $sql = 'SELECT uid FROM {coregrounds_app_session} WHERE token = :sid';
    if ($uid = db_query($sql, array(':sid' => $sid))->fetchField()) {
      $account = user_load($uid);

      if (empty($account->data->region)) {
        $output['message'] = 'Please choose a region in the settings.';
        drupal_json_output($output);
        drupal_exit();
      }

      $output['sid'] = $sid;
      $output['name'] = check_plain($account->name);
      $output['uid'] = intval($account->uid);
      $output['color'] = intval($account->data->color);
      $output['playerIcon'] = intval($account->data->emblem);
      $output['botLevel'] = intval($account->data->solo_progress);
      $output['region'] = $account->data->region;
      $output['crossregion'] = intval($account->data->crossregion);
      $output['level'] = intval($account->data->lvl);
      $output['unlocks'] = array_values(array_unique(array_merge(
        array_map('intval', $account->unlocks),
        coregrounds_get_free_dailies()
      )));

      // Add seasonal info for matchmaking.
      $output['season']['bot']['wins'] = intval($account->season->bot_wins);
      $output['season']['bot']['games'] = intval($account->season->bot_games);
      $output['season']['bot']['rank'] = intval($account->season->bot_rank);
      $output['season']['casual']['wins'] = intval($account->season->casual_wins);
      $output['season']['casual']['games'] = intval($account->season->casual_games);
      $output['season']['casual']['rank'] = intval($account->season->casual_rank);
      $output['season']['ranked']['wins'] = intval($account->season->ranked_wins);
      $output['season']['ranked']['games'] = intval($account->season->ranked_games);
      $output['season']['ranked']['rank'] = intval($account->season->ranked_rank);

      // Add skins as an array of skin ids.
      $output['skins'] = $account->data->skins
        ? array_map('intval', json_decode($account->data->skins))
        : array();

      // Add modification pages.
      $sql = 'SELECT * FROM coregrounds_modifications WHERE uid = :uid';
      foreach (db_query($sql, array(':uid' => $uid)) as $record) {
        $output['pages'][] = array(
          'pid' => intval($record->pid),
          'title' => check_plain($record->title),
          'mods' => $record->mods !== NULL
            ? json_decode($record->mods, true)
            : array(),
        );
      }
      $output['lastPage'] = $account->data->last_used_page
        ? intval($account->data->last_used_page)
        : NULL;
    } else {
      $output['message'] = 'Invalid session token.';
    }
  } else {
    $output['message'] = 'Invalid API key.';
  }
  drupal_json_output($output);
  drupal_exit();
}
