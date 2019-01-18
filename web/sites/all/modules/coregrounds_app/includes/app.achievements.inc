<?php

/**
 * @file
 * Handles achievements.
 */

/**
 * API callback returning the scoregrounds.
 */
function coregrounds_app_achievements($uid) {
  if ($account = user_load($uid)) {
    $output['success'] = 1;

    // Get this users achievemnts.
    coregrounds_get_achievements($account);

    $output['progress'] = $account->data->achievement_progress;

    // Prepare achievement output.
    foreach (configQuery('achievements') as $achievement) {
      // Get progress.
      $progress = isset($account->achievements[$achievement['id']])
        ? intval($account->achievements[$achievement['id']]->progress)
        : 0;
      // Add achievement to output.
      $output['achievements'][$achievement['id']] = $progress;
    }
  }
  else {
    $output['success'] = 0;
    $output['message'] = 'User not found';
  }

  coregrounds_app_api_response($output);
}
