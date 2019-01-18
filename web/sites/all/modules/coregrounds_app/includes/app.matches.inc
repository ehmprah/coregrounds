<?php

/**
 * @file
 * Provides data for the dashboard.
 */

/**
 * API callback returning the scoregrounds.
 */
function coregrounds_app_match($mid) {
  if ($match = coregrounds_load_match($mid)) {
    $output['success'] = 1;
    $output['data'] = $match;
  }
  else {
    $output['success'] = 0;
    $output['message'] = 'Match not found';
  }
  coregrounds_app_api_response($output);
}

/**
 * API callback returning the scoregrounds.
 */
function coregrounds_app_match_history($uid) {
  if ($account = user_load($uid)) {
    $output['success'] = 1;
    $output['data'] = array();
    // Load 10 most recent matches.
    $q = db_select('coregrounds_matches', 'm');
    $q->join('coregrounds_matches_players', 'p', 'p.mid = m.mid AND p.uid = :uid',
      array(':uid' => $account->uid));
    $q->fields('m', array('mid', 'match_type', 'won', 'duration', 'ended'));
    $q->fields('p', array('player', 'coins', 'xp', 'team'));
    $q->orderBy('m.ended', 'DESC');
    $q->range(0, 10);

    $sql = "SELECT id FROM coregrounds_matches_factories
            WHERE mid = :mid AND player = :pid
            UNION
            SELECT id FROM coregrounds_matches_abilities
            WHERE mid = :mid AND player = :pid";

    foreach ($q->execute() as $match) {
      // Add picks for this match.
      $match->picks = db_query($sql, array(
        ':mid' => $match->mid,
        ':pid' => $match->player,
      ))->fetchCol();
      // Add the match to the output.
      $output['data'][] = $match;
    }
  }
  else {
    $output['success'] = 0;
    $output['message'] = 'User not found';
  }

  coregrounds_app_api_response($output);
}
