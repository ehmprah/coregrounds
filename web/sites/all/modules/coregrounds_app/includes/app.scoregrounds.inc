<?php

/**
 * @file
 * Provides data for the dashboard.
 */

/**
 * API callback returning the scoregrounds.
 */
function coregrounds_app_scoregrounds() {

  $output['success'] = 1;

  foreach (array('bot', 'casual', 'ranked', 'level', 'achievements') as $type) {

    // Add tables.
    $query = db_select('users', 'u');
    $query->join('coregrounds_players', 'p', 'p.uid = u.uid');
    $query->join('coregrounds_seasons', 's', 's.uid = u.uid');

    // Restrict to current season.
    // TODO add lookup for old seasons as well.
    $query->where('s.season = :season', array(
      ':season' => variable_get('coregrounds_current_season'),
    ));

    // Add fields.
    $query->fields('u', array('name', 'uid'));
    $query->fields('p', array('lvl', 'achievement_progress'));
    $query->fields('s', array(
      'bot_wins',
      'bot_games',
      'bot_rank',
      'casual_wins',
      'casual_games',
      'ranked_wins',
      'ranked_games',
      'ranked_rank',
    ));

    // Add limit.
    $query->range(0, 100);

    // CASUAL ordering.
    if ($type == 'bot') {
      $query->orderBy('s.bot_rank', 'DESC');
      $query->orderBy('s.bot_winrate', 'DESC');
      $query->orderBy('s.bot_wins', 'DESC');
    }

    // CASUAL ordering.
    if ($type == 'casual') {
      $query->orderBy('s.casual_rank', 'DESC');
      $query->orderBy('s.casual_winrate', 'DESC');
      $query->orderBy('s.casual_wins', 'DESC');
    }

    // RANKED ordering.
    if ($type == 'ranked') {
      $query->orderBy('s.ranked_rank', 'DESC');
      $query->orderBy('s.ranked_winrate', 'DESC');
      $query->orderBy('s.ranked_wins', 'DESC');
    }

    // ACHIEVEMENTS ordering.
    if ($type == 'achievements') {
      $query->orderBy('p.achievement_progress', 'DESC');
    }

    // CREDITS ordering.
    if ($type == 'level') {
      $query->orderBy('p.lvl', 'DESC');
    }

    foreach ($query->execute() as $player) {
      $output['data'][$type][] = array(
        'uid' => $player->uid,
        'name' => $player->name,
        'bot' => $player->bot_rank,
        'casual' => $player->casual_wins . ' / ' . ($player->casual_games - $player->casual_wins),
        'ranked' => $player->ranked_wins . ' / ' . ($player->ranked_games - $player->ranked_wins),
        'rank' => 50 - floor($player->ranked_rank / 5),
        'level' => $player->lvl,
        'achievements' => number_format($player->achievement_progress, 4),
      );
    }
  }

  // Send response.
  coregrounds_app_api_response($output);
}

/**
 * API callback returning the scoregrounds.
 */
function coregrounds_app_player_profile($uid) {
  if ($account = user_load($uid)) {
    $output['success'] = 1;
    $output['profile']['name'] = check_plain($account->name);
    $output['profile']['icon'] = intval($account->data->emblem);
    $output['profile']['achievementProgress'] = floatval($account->data->achievement_progress);
    $output['profile']['winStreak'] = intval($account->data->pvp_winstreak);
    $output['profile']['unlocks'] = $account->unlocks;
    $output['profile']['level']['current'] = intval($account->data->lvl);
    $output['profile']['level']['progress'] = floatval($account->data->level_progress);
    $output['profile']['credits']['earned'] = intval($account->data->coins_earned);
    foreach (['season', 'previous'] as $key) {
      foreach(['bot', 'casual', 'ranked'] as $type) {
        $output['profile'][$key][$type]['wins'] = intval($account->{$key}->{$type . '_wins'});
        $output['profile'][$key][$type]['games'] = intval($account->{$key}->{$type . '_games'});
        $output['profile'][$key][$type]['rank'] = intval($account->{$key}->{$type . '_rank'});
      }
    }
    return coregrounds_app_api_response($output);
  }
  return MENU_NOT_FOUND;
}
