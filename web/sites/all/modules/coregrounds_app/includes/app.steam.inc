<?php

/**
 * API callback authenticating a user against the steam servers
 */
function coregrounds_app_steam_auth() {
  $output['success'] = 0;
  // Authenticate the ticket against the steam servers
  if ($steamid = _authenticate_user_ticket($_POST['ticket'])) {
    $uid = _get_linked_uid($steamid);
    if (variable_get('coregrounds_app_maintenance', 0) && !in_array($uid, [1, 64, 189])) {
      return coregrounds_app_api_response([
        'maintenance' => 1,
      ]);
    }
    $output['success'] = 1;
    // Create session token.
    if ($uid) {
      $output += _get_session_data($uid, TRUE);
    }
  } else {
    $output['message'] = 'Authentication failed.';
  }
  coregrounds_app_api_response($output);
}

/**
 * API callback linking steam account to existing coregrounds account
 */
function coregrounds_app_steam_link() {
  $output['success'] = 0;
  // Authenticate the ticket against the steam servers
  if ($steamid = _authenticate_user_ticket($_POST['ticket'])) {
    // If username is an email address, load the name first.
    $username = $_POST['username'];
    if (valid_email_address($_POST['username'])) {
      if ($account = user_load_by_mail($_POST['username'])) {
        $username = $account->name;
      }
    }
    $password = trim($_POST['password']);
    // Validate and save session.
    if ($uid = user_authenticate($username, $password)) {
      $output['success'] = 1;
      // Now link the account
      db_update('coregrounds_players')->fields([
        'steamid' => $steamid
      ])->condition('uid', $uid)->execute();
      // Wrap up and finish auth
      $output += _get_session_data($uid, TRUE);
    } else {
      $output['message'] = 'Wrong username or password.';
    }
  } else {
    $output['message'] = 'Authentication failed.';
  }
  coregrounds_app_api_response($output);
}

/**
 * API callback creating a new account for a steam id
 */
function coregrounds_app_steam_init() {
  $output['success'] = 0;
  // Check if a user with this name already exists.
  if (user_load_by_name($_POST['username'])) {
    $output['message'] = 'This username is already taken.';
  } else if ($error = user_validate_name($_POST['username'])) {
    $output['message'] = $error;
  } else if ($steamid = _authenticate_user_ticket($_POST['ticket'])) {
    // Build the account data.
    $raw = array(
      'name' => $_POST['username'],
      // We add the steam id as email adress
      'mail' => $steamid . '@coregrounds.com',
      // We add a random password
      'pass' => substr(md5(microtime()), rand(0, 26), 5),
      'status' => 1,
      'access' => REQUEST_TIME,
      'roles' => array(),
    );
    // Save account.
    if ($account = user_save(NULL, $raw)) {
      $output['success'] = 1;
      // Now link the account
      $query = db_update('coregrounds_players');
      $query->fields([
        'steamid' => $steamid,
        // Steam initially shows the ToS, so we can mark these as accepted for new users
        'tos_accepted' => variable_get('coregrounds_tos_version', '0.0.0'),
      ]);
      // Users get 7500 credits at the beginning to unlock three mods and thre units
      $query->expression('coins', 'coins + :coins', array(':coins' => 10500));
      $query->condition('uid', $account->uid)->execute();
      // Wrap up and finish auth
      $output += _get_session_data($account->uid, TRUE);
    } else {
      $output['message'] = 'Error creating account.';
    }
  } else {
    $output['message'] = 'Authentication failed.';
  }
  coregrounds_app_api_response($output);
}

/**
 * Helper function validating a user session ticket against the steam api
 */
function _authenticate_user_ticket($ticket) {
  $url = 'https://partner.steam-api.com/ISteamUserAuth/AuthenticateUserTicket/v1/';
  $url .= '?key=' . STEAM_API_KEY;
  $url .= '&appid=649770';
  $url .= '&ticket=' . check_plain($ticket);
  if ($response = file_get_contents($url)) {
    if ($parsed = json_decode($response, TRUE)) {
      if (isset($parsed['response']['params']['steamid'])) {
        return $parsed['response']['params']['steamid'];
      }
    }
  }
  return false;
}

/**
 * Helper function looking for a linked coregrounds account for a given steam id
 */
function _get_linked_uid($steamid) {
  $query = "SELECT uid FROM coregrounds_players WHERE steamid = :id";
  if ($uid = db_query($query, [':id' => $steamid])->fetchField()) {
    return $uid;
  }
  return false;
}
