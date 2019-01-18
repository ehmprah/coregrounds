<?php

/**
 * Analytics
 *
 * TODO add table to save that once a day
 * TODO add it retroactively from april 30
 * TODO add timepicker to select days in the past
 */
function coregrounds_analytics() {
  $to = REQUEST_TIME;
  if (isset($_GET['time'])) {
    if ($custom = strtotime($_GET['time'])) {
      $to = $custom;
    }
  }

  $day_start = strtotime("yesterday 00:00", $to);
  $day_end = strtotime("yesterday 23:59", $to);

  $month_start = strtotime("first day of previous month 00:00");
  $month_end = strtotime("last day of previous month 23:59");

  $day = [
    ':time_from' => $day_start,
    ':time_to' => $day_end,
  ];

  // Get total users
  $q = "SELECT COUNT(*) FROM users WHERE created < :to";
  $analytics['users_total'] = db_query($q, [':to' => $to])->fetchField();

  // Get new users
  $q = "SELECT COUNT(*) FROM users WHERE created BETWEEN :time_from AND :time_to";
  $analytics['users_new'] = db_query($q, $day)->fetchField();

  // Get daily active users
  $q = "SELECT COUNT(DISTINCT uid) FROM coregrounds_matches_players p
        JOIN coregrounds_matches m ON p.mid = m.mid AND m.ended BETWEEN :time_from AND :time_to";
  $analytics['dau'] = db_query($q, $day)->fetchField();

  // Get monthly active users
  $q = "SELECT COUNT(DISTINCT uid) FROM coregrounds_matches_players p
        JOIN coregrounds_matches m ON p.mid = m.mid AND m.ended BETWEEN :time_from AND :time_to";
  $analytics['mau'] = db_query($q, [
    ':time_from' => $month_start,
    ':time_to' => $month_end,
  ])->fetchField();

  // Retention
  $q = "SELECT COUNT(DISTINCT u.uid) FROM users u
        JOIN coregrounds_matches_players p ON p.uid = u.uid
        JOIN coregrounds_matches m ON p.mid = m.mid AND m.ended > u.created + :period";
  $analytics['d1'] = db_query($q, [
    ':period' => 86400
  ])->fetchField() / $analytics['users_total'] * 100;
  $analytics['d7'] = db_query($q, [
    ':period' => 7 * 86400
  ])->fetchField() / $analytics['users_total'] * 100;
  $analytics['d28'] = db_query($q, [
    ':period' => 28* 86400
  ])->fetchField() / $analytics['users_total'] * 100;

  // Matches per day
  $q = "SELECT match_type, count(match_type) FROM coregrounds_matches
        WHERE ended BETWEEN :time_from AND :time_to GROUP BY match_type";
  $analytics['matches'] = db_query($q, $day)->fetchAllKeyed();

  // Total bot games against level 1
  $q = "SELECT COUNT(*) FROM coregrounds_matches m
        JOIN coregrounds_matches_players p ON m.mid = p.mid AND p.name = 'Bot (Level 1)'
        WHERE m.match_type = 0 AND m.ended BETWEEN :time_from AND :time_to";
  if ($first_bot_matches = db_query($q, $day)->fetchField()) {
    // Lost bot games
    $q = "SELECT COUNT(*) FROM coregrounds_matches m
          JOIN coregrounds_matches_players p ON m.mid = p.mid AND p.name = 'Bot (Level 1)'
          WHERE m.match_type = 0 AND m.won = 2 AND m.ended BETWEEN :time_from AND :time_to";
    $analytics['first_bot_lost'] = db_query($q, $day)->fetchField() / $first_bot_matches * 100;
    // Surrendered bot games
    $q = "SELECT COUNT(*) FROM coregrounds_matches m
          JOIN coregrounds_matches_players p1 ON m.mid = p1.mid AND p1.name = 'Bot (Level 1)'
          JOIN coregrounds_matches_players p2 ON m.mid = p2.mid AND p2.team = 1 AND p2.core_health > 0
          WHERE m.match_type = 0 AND m.won = 2 AND m.ended BETWEEN :time_from AND :time_to";
    $analytics['first_bot_surrender'] = db_query($q, $day)->fetchField() / $first_bot_matches * 100;
  } else {
    $analytics['first_bot_lost'] = 0;
    $analytics['first_bot_surrender'] = 0;
  }

  // Bot games won against lvl
  $q = "SELECT COUNT(DISTINCT p2.uid) FROM coregrounds_matches m
        JOIN coregrounds_matches_players p1 ON m.mid = p1.mid AND p1.name = :lvl
        JOIN coregrounds_matches_players p2 ON m.mid = p2.mid AND p2.team = 1
        WHERE m.match_type = 0 AND m.won = 1";
  foreach ([1, 2, 5, 10, 25] as $lvl) {
    $players = db_query($q, [':lvl' => "Bot (Level $lvl)"])->fetchField();
    $analytics['bot_stats'][] = [
      'level' => $lvl,
      'ratio' => $players / $analytics['users_total'] * 100,
    ];
  }

  // Subscribers
  $q = "SELECT COUNT(*) FROM users u
        JOIN field_data_field_newsletter f ON u.uid = f.entity_id AND f.field_newsletter_value = 1
        WHERE u.created < :to";
  $analytics['subscribers'] = db_query($q, [
    ':to' => $to,
  ])->fetchField() / $analytics['users_total'] * 100;

  // Conversions
  $q = "SELECT COUNT(DISTINCT uid) FROM coregrounds_transactions
        WHERE STATUS = :status AND created < :to";
  $analytics['conversions'] = db_query($q, [
    ':status' => 'finalized',
    ':to' => $to,
  ])->fetchField() / $analytics['users_total'] * 100;

  return array(
    '#theme' => 'analytics',
    '#analytics' => $analytics,
  );
}
