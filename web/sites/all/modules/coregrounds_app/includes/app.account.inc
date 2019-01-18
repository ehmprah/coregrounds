<?php

/**
 * API callback reloading player data
 */
function coregrounds_app_reload_player() {
  $output['success'] = 0;
  // For everything else we have to validate the session first.
  if ($uid = coregrounds_app_validate_session()) {
    $output['success'] = 1;
    $output['player'] = coregrounds_app_build_player_data($uid);
    $output['achievements'] = coregrounds_get_unread_achievements($uid);
  }
  coregrounds_app_api_response($output);
}

/**
 * API callback saving the tutorial completion for a user.
 */
function coregrounds_app_tos_agreement() {
  $output['success'] = 0;
  // For everything else we have to validate the session first.
  if ($uid = coregrounds_app_validate_session()) {
    // Save the tutorial completion in the player data.
    db_update('coregrounds_players')->fields(array(
      'tos_accepted' => variable_get('coregrounds_tos_version', '0.0.0'),
    ))->condition('uid', $uid)->execute();
    $output['success'] = 1;
  }
  coregrounds_app_api_response($output);
}

/**
 * API callback saving the tutorial completion for a user.
 */
function coregrounds_app_save_region() {
  $output['success'] = 0;
  // For everything else we have to validate the session first.
  if ($uid = coregrounds_app_validate_session()) {
    if (in_array($_POST['region'], ['local', 'us', 'europe', 'asia'])) {
      db_update('coregrounds_players')->fields(array(
        'region' => $_POST['region'],
        'crossregion' => $_POST['crossregion'],
      ))->condition('uid', $uid)->execute();
      $output['success'] = 1;
    } else {
      $output['message'] = 'Invalid region.';
    }
  }
  coregrounds_app_api_response($output);
}

/**
 * API callback saving the tutorial completion for a user.
 */
function coregrounds_app_complete_tutorial() {
  $output['success'] = 0;
  // For everything else we have to validate the session first.
  if ($uid = coregrounds_app_validate_session()) {
    // Save the tutorial completion in the player data.
    db_update('coregrounds_players')->fields(array(
      'tutorial' => 1,
    ))->condition('uid', $uid)->execute();
    $output['success'] = 1;

    // Update and attach achievement.
    $account = user_load($uid);
    coregrounds_update_achievements($account, 1, array(1));
    $output['achievements'] = coregrounds_get_unread_achievements($account->uid);
  }
  coregrounds_app_api_response($output);
}

/**
 * API callback unlocking an item
 */
function coregrounds_app_save_modifications() {
  $output['success'] = 0;

  // Decode the json
  $page = json_decode($_POST['page']);

  // We can immediately check for an empty title
  if (empty($page->title)) {
    $output['message'] = 'Page title cannot be empty.';
    coregrounds_app_api_response($output);
  }

  // For everything else we have to validate the session first
  if ($uid = coregrounds_app_validate_session()) {
    $account = user_load($uid);
    foreach ($page->mods as $type) {
      foreach ($type as $mod) {
        if ($mod) {
          $modification = configQuery('modifications', 'id', $mod);
          if (!in_array($modification['id'], $account->unlocks)) {
            $output['message'] = 'Invalid modification selection.';
            coregrounds_app_api_response($output);
          }
        }
      }
    }

    // If the page survived the validation so far, we can save it
    db_update('coregrounds_modifications')->fields(array(
      "title" => $page->title,
      "mods" => json_encode($page->mods),
    ))->condition('pid', $page->pid)->condition('uid', $uid)->execute();

    // And display a success messsage to the user
    $output['success'] = 1;
    $output['message'] = 'Modification page saved!';
    coregrounds_app_api_response($output);
  }
}

/**
 * App API callback changing a user's color
 */
function coregrounds_app_customize() {
  if ($uid = coregrounds_app_validate_session()) {
    $fields = array();
    // TODO load the unlocks differently once we updated the player load mechanism
    $account = user_load($uid);

    // Save color id
    if (isset($_POST['colorId'])) {
      if (in_array($_POST['colorId'], $account->unlocks)) {
        $fields['color'] = $_POST['colorId'];
      }
    }

    // Save player icon
    if (isset($_POST['playerIcon'])) {
      if (in_array($_POST['playerIcon'], $account->unlocks)) {
        $fields['emblem'] = $_POST['playerIcon'];
      }
    }

    // Save background
    if (isset($_POST['background'])) {
      if ($_POST['background'] == 0 || in_array($_POST['background'], $account->unlocks)) {
        $fields['background'] = $_POST['background'];
      }
    }

    // Save skin selection
    if (isset($_POST['skins'])) {
      $skins = array();
      foreach (json_decode($_POST['skins']) as $skin) {
        if (in_array($skin, $account->unlocks)) {
          $skins[] = $skin;
        }
      }
      $fields['skins'] = json_encode($skins);
    }

    if ($fields) {
      // Update the database
      db_update('coregrounds_players')
        ->fields($fields)
        ->condition('uid', $uid)
        ->execute();
    }

    // Return success message
    $output['success'] = 1;
    $output['message'] = 'Settings saved';
  } else {
    $output['success'] = 0;
    $output['message'] = 'Invalid session token';
  }

  coregrounds_app_api_response($output);
}

/**
 * App API callback upgrading an account
 */
function coregrounds_app_account_upgrade() {
  $output['success'] = 0;
  if ($uid = coregrounds_app_validate_session()) {
    $account = user_load($uid);
    if ($account->login) {
      $output['message'] = 'This account already is linked to a Coregrounds account.';
    } else if (!valid_email_address($_POST['mail'])) {
      $output['message'] = 'Invalid email address';
    } else if (user_load_by_mail($_POST['mail'])) {
      $output['message'] = 'This email address is already linked to another account.';
    } else {
      // Update the user's email address
      $account->mail = $_POST['mail'];
      user_save($account);
      // Send verification email
      _user_mail_notify('register_no_approval_required', $account);
      $output['success'] = 1;
      $output['message'] = 'Success! Please check your mail for further instructions!';
    }
  } else {
    $output['message'] = 'Invalid session token';
  }
  coregrounds_app_api_response($output);
}

/**
 * App API callback (un-)subscribing users to newsletter
 */
function coregrounds_app_account_subscribe() {
  $output['success'] = 0;
  if ($uid = coregrounds_app_validate_session()) {
    $account = user_load($uid);
    $account->field_newsletter['und'][0]['value'] = $_POST['subscribe'] ? 1 : 0;
    user_save($account);
    $output['success'] = 1;
    $output['message'] = 'Settings saved!';
  } else {
    $output['message'] = 'Invalid session token';
  }
  coregrounds_app_api_response($output);
}

/**
 * Callback redeeming coupon codes
 */
function coregrounds_app_redeem() {
  $output['success'] = 0;
  if ($uid = coregrounds_app_validate_session()) {
    if (!flood_is_allowed('redeem_code', 5, 86400)) {
      $output['message'] = 'Too many failed attempts.';
    } else {
      $q = "SELECT * FROM coregrounds_coupons WHERE code = :code";
      $code = db_query($q, array(':code' => $_POST['code']))->fetchObject();
      if ($code) {
        if ($code->used_by) {
          $output['message'] = 'This code has already been used.';
        } else {
          // Update player
          $query = db_update('coregrounds_players');
          $query->expression('pc_current', 'pc_current + :cordium', [':cordium' => $code->cordium]);
          $query->condition('uid', $uid)->execute();
          // Update coupon
          db_update('coregrounds_coupons')->fields(array(
            'used_by' => $uid,
          ))->condition('code', $code->code)->execute();
          // Display message
          $output['success'] = 1;
          $output['message'] = 'Code redeemed, ' . $code->cordium . ' Cordium granted!';
          $output['cordium'] = $code->cordium;
        }
      } else {
        $output['message'] = 'Invalid code.';
      }
      flood_register_event('redeem_code', 86400);
    }
  } else {
    $output['message'] = 'Invalid session token';
  }
  coregrounds_app_api_response($output);
}
