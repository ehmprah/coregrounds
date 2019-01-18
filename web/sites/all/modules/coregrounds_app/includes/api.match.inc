<?php

/**
 * @file
 * Handles match statistics.
 */

/**
 * Handles finished match, saving statistics and more.
 */
function coregrounds_api_match() {

  $matchTypes = array(
    'bot' => 0,
    'casual' => 1,
    'ranked' => 2,
    'private' => 3,
    'random' => 4,
    'custombot' => 5,
  );

  $output['success'] = 0;
  if (isset($_POST['key']) && $_POST['key'] === COREGROUNDS_APP_KEY) {

    // Save the general match info and get a match id.
    $mid = db_insert('coregrounds_matches')->fields(array(
      'match_type' => $matchTypes[$_POST['match']['match_type']],
      'won' => $_POST['match']['winner_team'],
      'ended' => REQUEST_TIME,
      'duration' => $_POST['match']['duration'],
    ))->execute();

    // Loop over players, handling them one at a time.
    foreach ($_POST['match']['players'] as $playerIndex => $player) {
      $rewards['credits'] = 0;
      $rewards['xp'] = 0;

      if (isset($player['uid'])) {
        // Load the player and attach achievements.
        $account = user_load($player['uid']);
        coregrounds_get_achievements($account);
        // Set variables for the match player table
        $uid = $player['uid'];
        $name = check_plain($account->name);
        $color = $account->data->color;

        // In case of casual or ranked games we save statistics.
        if (in_array($_POST['match']['match_type'], ['bot', 'casual', 'ranked'])) {
          _save_season_stats(
            $account,
            $_POST['match']['match_type'],
            $player['team_id'] == $_POST['match']['winner_team']
          );
        }

        // Get the xp and credit rewards and handle achievements.
        $rewards = _get_match_rewards($account, $player, $_POST['match']);
      } else {
        $uid = 0;
        $name = 'Bot (Level ' . $player['bot_difficulty'] . ')';
        $color = 2401;
      }

      // Save the match players.
      db_insert('coregrounds_matches_players')->fields(array(
        'mid' => $mid,
        'player' => $playerIndex,
        'team' => $player['team_id'],
        'uid' => $uid,
        'name' => $name,
        'color' => $color,
        'minions_spawned' => $player['minions_spawned'],
        'walls_built' => $player['walls_built'],
        'walls_killed' => $player['walls_killed'],
        'towers_killed' => $player['towers_killed'],
        'damage' => $player['damage'],
        'healing' => $player['healing'],
        'core_health' => $player['core_health'],
        'coins' => $rewards['credits'],
        'xp' => $rewards['xp'],
        'ban_1' => (isset($player['factory_bans'][0])
          ? $player['factory_bans'][0]
          : 0),
        'ban_2' => (isset($player['ability_bans'][0])
          ? $player['ability_bans'][0]
          : 0),
      ))->execute();

      // Save factories.
      $factories = db_insert('coregrounds_matches_factories')->fields([
        'mid',
        'player',
        'id',
        'pickorder',
        'xp',
        'upgrade_1',
        'upgrade_2',
        'upgrade_3',
      ]);
      foreach ($player['factories'] as $index => $factory) {
        // We don't save the wall factory.
        if ($factory['id'] > 1) {
          $factories->values(array(
            'mid' => $mid,
            'player' => $playerIndex,
            'id' => $factory['id'],
            'pickorder' => $index,
            'xp' => $factory['xp'],
            'upgrade_1' => $factory['upgrades'][0],
            'upgrade_2' => $factory['upgrades'][1],
            'upgrade_3' => $factory['upgrades'][2],
          ));
        }
      }
      $factories->execute();

      // Save abilities.
      $abilities = db_insert('coregrounds_matches_abilities')->fields([
        'mid',
        'player',
        'id',
        'pickorder',
        'uses',
      ]);
      foreach ($player['abilities'] as $index => $factory) {
        $abilities->values(array(
          'mid' => $mid,
          'player' => $playerIndex,
          'id' => $factory['id'],
          'pickorder' => $index,
          'uses' => $factory['uses'],
        ));
      }
      $abilities->execute();
    }

    // Save draft stats for certain match types.
    if (in_array($_POST['match']['match_type'], ['bot', 'casual', 'ranked'])) {
      _save_draft_stats($_POST['match']);
    }

    $output['success'] = 1;
    $output['match_id'] = intval($mid);
  } else {
    $output['message'] = 'Invalid API key.';
  }
  // Send response.
  drupal_json_output($output);
  drupal_exit();
}

/**
 * Updates a player's seasonal stats.
 */
function _save_season_stats($account, $type, $won) {
  // Build the values to update.
  $games = $account->season->{"{$type}_games"} + 1;
  if ($won) {
    $wins = $account->season->{"{$type}_wins"} + 1;
    $rank = $account->season->{"{$type}_rank"} + 1;
    // For bot games, we cap the rank at 100.
    if ($type == 'bot' && $rank > 100) {
      $rank = 100;
    }
  } else {
    $wins = $account->season->{"{$type}_wins"};
    // Decrement the rank.
    $rank = $account->season->{"{$type}_rank"} - 1;
    // Except for bot games.
    if ($type == 'bot') {
      $rank = $account->season->{"{$type}_rank"};
    }
    // Also, we don't allow negative ranks.
    if ($rank < 0) {
      $rank = 0;
    }
  }
  // Write update to the database.
  db_update('coregrounds_seasons')
    ->fields([
      "{$type}_games" => $games,
      "{$type}_wins" => $wins,
      "{$type}_rank" => $rank,
      "{$type}_winrate" => ($wins / $games),
    ])
    ->condition('uid', $account->uid)
    ->condition('season', variable_get('coregrounds_current_season'))
    ->execute();
}

/**
 * Calculate a player's credit and xp reward for a match and achievements.
 */
function _get_match_rewards($account, $player, $match) {
  $won = $player['team_id'] == $match['winner_team'];

  // You only get rewards for a maximum of five private matches per day.
  if ($match['match_type'] == 'private' || $match['match_type'] == 'randomized') {
    if ($account->data->pmatches >= 5) {
      if ($won) {
        $fields['wins'] = $account->data->wins + 1;
      }
      $fields['last_used_page'] = $player['modpage_index'];
      coregrounds_update_player($account, $fields);
    }
  }

  // Sum upgrades and xp for the achievements and credit calculation.
  $total_upgrades = 0;
  $total_xp = 0;

  foreach ($player['factories'] as $factory) {
    $total_upgrades += array_sum($factory['upgrades']);
    $total_xp += $factory['xp'];
  }

  $total_abilities = 0;
  foreach ($player['abilities'] as $ability) {
    $total_abilities += $ability['uses'];
  }

  // Calculate experience.
  $rewards['xp'] = 0;
  $rewards['xp'] += _get_capped_val(ceil($total_xp / 1000), 5);
  $rewards['xp'] += _get_capped_val(ceil($player['damage'] / 1000), 6);
  $rewards['xp'] += _get_capped_val(ceil($player['minions_killed'] / 100), 10);
  $rewards['xp'] += _get_capped_val(ceil($player['minions_spawned'] / 50), 8);
  $rewards['xp'] += _get_capped_val(ceil($player['walls_built'] / 5), 5);
  $rewards['xp'] += _get_capped_val(ceil($player['walls_killed'] / 15), 7);
  $rewards['xp'] += _get_capped_val(ceil($total_upgrades / 5), 5);
  $rewards['xp'] += _get_capped_val(ceil($total_abilities / 5), 5);
  if ($won) {
    $rewards['xp'] += 10;
  }

  // PVP matches yield more credits
  if ($match['match_type'] != 'bot' && $match['match_type'] != 'bot') {
    $rewards['xp'] += floor($rewards['xp'] * 0.2);
  }

  // Calculate base credits.
  $credits = 0;
  $credits += _get_capped_val(ceil($total_xp / 1500), 5);
  $credits += _get_capped_val(ceil($player['damage'] / 5000), 5);
  $credits += _get_capped_val(ceil($player['walls_built'] / 5), 5);
  $credits += _get_capped_val(ceil($total_upgrades / 5), 5);
  $credits += _get_capped_val(ceil($total_upgrades / 5), 5);
  $credits += _get_capped_val(ceil($total_abilities / 5), 5);
  if ($won) {
    $credits += 10;
  }

  // PVP matches yield more credits
  if ($match['match_type'] != 'bot' && $match['match_type'] != 'bot') {
    $credits += floor($credits * 0.2);
  }

  // Add the base amount
  $rewards['credits'] = $credits;

  // First win of the day.
  $rewards['credits'] += $won
    ? floor($credits * $account->data->daily_bonus / 100)
    : 0;

  // Boost per win.
  if ($won && $account->data->boost_wins > 0) {
    $rewards['credits'] += $credits;
    $fields['boost_wins'] = $account->data->boost_wins - 1;
  }

  // Boost over time.
  if ($account->data->boost_until > REQUEST_TIME) {
    $rewards['credits'] += $credits;
  }

  // Permaboost.
  if ($account->data->permaboost > 0 || $account->field_newsletter['und'][0]['value']) {
    $perma_base = $account->data->permaboost;
    if ($account->field_newsletter['und'][0]['value']) {
      $perma_base += 50;
    }
    $rewards['credits'] += floor($credits * ($perma_base / 100));
  }

  // Respect global bonus.
  if (variable_get('coregrounds_global_bonus', FALSE)) {
    $rewards['credits'] *= 2;
  }

  // Winstreaks.
  $fields['pvp_winstreak'] = $won
    ? $account->data->pvp_winstreak + 1
    : 0;

  // Wins.
  if ($won) {
    $fields['wins'] = $account->data->wins + 1;
  }

  // Set a few stanard fields.
  $fields['daily_match'] = 1;
  $fields['matches'] = $account->data->matches + 1;
  $fields['last_used_page'] = $player['modpage_index'];

  // Increase counter for private matches.
  if ($match['match_type'] == 'private' || $match['match_type'] == 'randomized') {
    $fields['pmatches'] = $account->data->pmatches + 1;
  }

  // Save botlevel
  if ($won && $match['match_type'] == 'bot') {
    // Increment the level we won against
    $fields['solo_progress'] = _get_capped_val($match['players'][1]['bot_difficulty'], 100);
    // Handle the bot level achievements
    coregrounds_update_achievements(
      $account,
      $match['players'][1]['bot_difficulty'],
      [206, 207, 208, 209],
      TRUE
    );
  }

  // Write changes to database.
  coregrounds_update_player(
    $account,
    $fields,
    $rewards['credits'],
    $rewards['xp']
  );

  // Winstreak achievement.
  if ($fields['pvp_winstreak']) {
    coregrounds_update_achievements($account, 1, [12, 13, 14, 15]);
  } else {
    coregrounds_update_achievements($account, 0, [12, 13, 14, 15], 1);
  }

  // Win / loss achievements.
  if ($won) {
    coregrounds_update_achievements($account, 1, [2, 70, 71, 72, 73, 74]);
  } else {
    coregrounds_update_achievements($account, 1, [3]);
  }

  // Reach rank achievements.
  if ($won && $match['match_type'] == 'ranked') {
    $rank = floor(($account->season->ranked_rank + 1) / 5);
    coregrounds_update_achievements($account, $rank, [65, 66, 67, 68, 69], TRUE);
  }

  if ($won) {
    // Rush achievement.
    if ($match['duration'] <= 120) {
      coregrounds_update_achievements($account, 1, [4]);
    }

    // All tower picks achievement.
    if (_all_picks_one_type($player['factories'], 'tower')) {
      coregrounds_update_achievements($account, 1, [5]);
    }

    // All minion picks achievement.
    if (_all_picks_one_type($player['factories'], 'minion')) {
      coregrounds_update_achievements($account, 1, [6]);
    }

    // Unharmed core achievement.
    if ($won && $player['core_health'] == 100) {
      coregrounds_update_achievements($account, 1, [7]);
    }

    // Comeback.
    if ($won && $player['core_health'] <= 25) {
      coregrounds_update_achievements($account, 1, [8]);
    }

    // Ultimate comeback.
    if ($won && $player['core_health'] <= 10) {
      coregrounds_update_achievements($account, 1, [9]);
    }

    // Open spaces.
    if ($player['walls_built'] == 0) {
      coregrounds_update_achievements($account, 1, [10]);
    }

    // Perfectionist.
    if ($total_upgrades == 60) {
      coregrounds_update_achievements($account, 1, [11]);
    }
  }

  // Minions spawned.
  coregrounds_update_achievements($account, $player['minions_spawned'], [45, 46, 47, 48, 49]);

  // Damage dealt.
  coregrounds_update_achievements($account, $player['damage'], [50, 51, 52, 53, 54]);

  // Mob kills.
  coregrounds_update_achievements($account, $player['minions_killed'],[55, 56, 57, 58, 59]);

  // Tower kills.
  coregrounds_update_achievements($account, $player['towers_killed'], [60, 61, 62, 63, 64]);

  // Wall build.
  coregrounds_update_achievements($account, $player['walls_built'],[30, 31, 32, 33, 34]);

  // Wall kills.
  coregrounds_update_achievements($account, $player['walls_killed'], [35, 36, 37, 38, 39]);

  // Upgrades.
  coregrounds_update_achievements($account, $total_upgrades, [25, 26, 27, 28, 29]);

  // Healing.
  coregrounds_update_achievements($account, $player['healing'], [40, 41, 42, 43, 44]);

  // Abilities.
  coregrounds_update_achievements($account, $total_abilities, [20, 21, 22, 23, 24]);

  // Damage dealt per match.
  coregrounds_update_achievements($account, $player['damage'], [301, 302, 303, 304, 305], 1, 0);

  // Xp gained per match.
  coregrounds_update_achievements($account, $total_xp, [306, 307, 308, 309, 310], 1, 0);

  // Xp gained total.
  coregrounds_update_achievements($account, $total_xp, [311, 312, 313, 314, 315]);

  return $rewards;
}

/**
 * Helper function saving the pick stats.
 */
function _save_draft_stats($match) {
  $patch = variable_get('coregrounds_version', '0.0.0');
  // Loop over players, handling them one at a time.
  foreach ($match['players'] as $playerIndex => $player) {
    // Don't record stats for bot players
    if (!isset($player['uid'])) continue;

    $won = $player['team_id'] == $match['winner_team'];
    $banned = array();
    $picked = array();
    // Add banned factories.
    if ($player['factory_bans']) {
      $banned += $player['factory_bans'];
    }
    if ($player['ability_bans']) {
      $banned += $player['ability_bans'];
    }
    // Add factories.
    foreach ($player['factories'] as $index => $factory) {
      // We don't save the wall factory.
      if ($factory['id'] > 1) {
        $picked[] = $factory['id'];
      }
    }
    foreach ($player['abilities'] as $index => $factory) {
      $picked[] = $factory['id'];
    }

    // Process picks.
    foreach ($picked as $unlid) {
      $query = db_update('coregrounds_statistics');
      $query->condition('unlid', $unlid);
      $query->condition('patch', $patch);
      $query->condition('season', variable_get('coregrounds_current_season'));
      $query->expression('picks', 'picks + :picks', array(':picks' => 1));
      if ($won) {
        $query->expression('wins', 'wins + :wins', array(':wins' => 1));
      }
      if ($query->execute() == 0) {
        db_insert('coregrounds_statistics')->fields(array(
          'unlid' => $unlid,
          'season' => variable_get('coregrounds_current_season'),
          'picks' => 1,
          'wins' => ($won ? 1 : 0),
          'patch' => $patch,
        ))->execute();
      }
    }

    // Process bans.
    foreach ($banned as $unlid) {
      $query = db_update('coregrounds_statistics');
      $query->condition('unlid', $unlid);
      $query->condition('patch', $patch);
      $query->condition('season', variable_get('coregrounds_current_season'));
      $query->expression('bans', 'bans + :bans', array(':bans' => 1));
      if ($query->execute() == 0) {
        db_insert('coregrounds_statistics')->fields(array(
          'unlid' => $unlid,
          'season' => variable_get('coregrounds_current_season'),
          'bans' => 1,
          'picks' => 0,
          'wins' => 0,
          'patch' => $patch,
        ))->execute();
      }
    }
  }
}

/**
 * Checks wether all picks are of one type.
 */
function _all_picks_one_type($factories, $type) {
  foreach ($factories as $factory) {
    // Skip the wall factory
    if ($factory['id'] == 1) continue;
    $item = configQuery('factories', 'id', $factory['id']);
    if (!isset($item[$type])) {
      return FALSE;
    }
  }
  return TRUE;
}

/**
 * Gets a value capped at a certain other value.
 */
function _get_capped_val($val, $cap) {
  return $val > $cap ? $cap : $val;
}
