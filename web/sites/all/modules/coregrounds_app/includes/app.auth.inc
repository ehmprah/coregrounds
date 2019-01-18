<?php

/**
 * API callback logging in to existing Coregrounds account
 */
function coregrounds_app_account_login() {
  $output['success'] = 0;
  // If username is an email address, load the name first.
  $username = $_POST['username'];
  if (valid_email_address($_POST['username'])) {
    if ($account = user_load_by_mail($_POST['username'])) {
      $username = $account->name;
    }
  }
  $password = trim($_POST['password']);
  if ($uid = user_authenticate($username, $password)) {
    $output['success'] = 1;
    $output += _get_session_data($uid, TRUE);
  } else {
    $output['message'] = 'Wrong username or password.';
  }
  coregrounds_app_api_response($output);
}

/**
 * API callback logging in to existing Coregrounds account
 */
function coregrounds_app_account_relog() {
  $output['success'] = 0;
  $q = "SELECT uid FROM coregrounds_app_session WHERE token = :token";
  if ($uid = db_query($q, [':token' => $_POST['token']])->fetchField()) {
    $output['success'] = 1;
    $output += _get_session_data($uid, TRUE, $_POST['token']);
  }
  coregrounds_app_api_response($output);
}

/**
 * API callback creating a new Coregrounds account
 */
function coregrounds_app_account_create() {
  $output['success'] = 0;
  // Check if a user with this name already exists.
  if (user_load_by_name($_POST['username'])) {
    $output['message'] = 'This username is already taken.';
  } else if ($error = user_validate_name($_POST['username'])) {
    $output['message'] = $error;
  } else {
    // Build the account data.
    $raw = array(
      'name' => $_POST['username'],
      // We add the steam id as email adress
      'name' => $_POST['mail'],
      // We add a random password
      'pass' => $_POST['pass'],
      'status' => 1,
      'access' => REQUEST_TIME,
      'roles' => array(),
    );
    // Save account.
    if ($account = user_save(NULL, $raw)) {
      $output['success'] = 1;
      $query = db_update('coregrounds_players');
      // Users get 7500 credits at the beginning to unlock three mods and thre units
      $query->expression('coins', 'coins + :coins', array(':coins' => 10500));
      $query->condition('uid', $account->uid)->execute();
      // Wrap up and finish auth
      $output += _get_session_data($account->uid, TRUE);
    } else {
      $output['message'] = 'Error creating account.';
    }
  }
  coregrounds_app_api_response($output);
}
