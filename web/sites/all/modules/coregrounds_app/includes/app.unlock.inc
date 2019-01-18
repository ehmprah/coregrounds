<?php

/**
 * @file
 * Provides data for the dashboard.
 */

/**
 * API callback unlocking an item.
 */
function coregrounds_app_buy_item() {
  $output['success'] = 0;
  // Validate player session.
  if ($uid = coregrounds_app_validate_session()) {
    // Load item from config.
    if ($item = configQuery($_POST['type'], 'id', $_POST['id'])) {
      $lock = 'transaction_' . $uid;
      $account = user_load($uid);
      if ($account->data->pc_current >= $item['price']) {
        if (lock_acquire($lock)) {
          // Deduct spent currency.
          $fields['pc_current'] = $account->data->pc_current - $item['price'];
          // Handle credit purchases.
          if ($_POST['type'] == 'credits') {
            if ($item['bonusWins']) {
              $fields['boost_wins'] = $account->data->boost_wins + $item['bonusWins'];
            }
            if ($item['bonusHours']) {
              $fields['boost_until'] = max($account->data->boost_until, REQUEST_TIME) + $item['bonusHours'] * 3600;
            }
            if ($item['bonusPermanent']) {
              $fields['permaboost'] = $account->data->permaboost + $item['bonusPermanent'];
            }
            if ($item['flatCredits']) {
              $fields['coins'] = $account->data->coins + $item['flatCredits'];
            }
            // Boost achievement.
            coregrounds_update_achievements($account, 1, array(95));
            // Treasure chest achievement
            if ((isset($fields['boost_wins']) && $account->data->boost_until > REQUEST_TIME) ||
                (isset($fields['boost_until']) && $account->data->wins > 0)) {
              coregrounds_update_achievements($account, 1, array(96));
            }
          } else {
            // All other purchases are handled via the unlock table.
            db_insert('coregrounds_unlocks')->fields(array(
              'uid' => $account->uid,
              'unlid' => $item['id'],
            ))->execute();
          }
          // Save changes to player table.
          db_update('coregrounds_players')
            ->fields($fields)
            ->condition('uid', $account->uid)
            ->execute();
          // Release the lock.
          lock_release($lock);
          // Return success message.
          $output['success'] = 1;
          $output['message'] = 'Purchase successful!';
          $output['player'] = coregrounds_app_build_player_data($uid);
          $output['achievements'] = coregrounds_get_unread_achievements($uid);
        }
      } else {
        $output['message'] = 'Insufficient funds.';
      }
    } else {
      $output['message'] = 'Not found: ' . $_POST['type'] . '/' . $_POST['id'];
    }
  }
  else {
    $output['message'] = 'Invalid session token';
  }

  coregrounds_app_api_response($output);
}

/**
 * API callback unlocking an item.
 */
function coregrounds_app_unlock_item() {
  $output['success'] = 0;
  // Validate player session.
  if ($uid = coregrounds_app_validate_session()) {
    $lock = 'unlock_transaction_' . $uid;
    $id = $_POST['id'];
    $account = user_load($uid);
    // Load unlock item from config.
    $item = configQuery([
      'factories',
      'abilities',
      'modifications',
    ], 'id', $id);
    if ($item) {
      if (!in_array($id, $account->unlocks)) {
        if ($account->data->coins >= $item['price']) {
          if (lock_acquire($lock)) {
            // Deduct credits.
            db_update('coregrounds_players')->fields(array(
              'coins' => ($account->data->coins - $item['price']),
            ))->condition('uid', $account->uid)->execute();
            // Save unlock.
            db_insert('coregrounds_unlocks')->fields(array(
              'uid' => $account->uid,
              'unlid' => $id,
            ))->execute();
            // Release the lock.
            lock_release($lock);
            // Update achievements.
            coregrounds_update_achievements($account, 1, [101, 102, 103, 104, 105]);
            // Return success message.
            $output['success'] = 1;
            $output['message'] = 'Unlock successful.';
            $output['player'] = coregrounds_app_build_player_data($uid);
            $output['achievements'] = coregrounds_get_unread_achievements($uid);
          }
        }
        else {
          $output['message'] = 'Insufficient credits.';
        }
      }
      else {
        $output['message'] = 'You already unlocked this item.';
      }
    }
    else {
      $output['message'] = 'Unlock not found.';
    }
  }
  else {
    $output['message'] = 'Invalid session token';
  }

  coregrounds_app_api_response($output);
}

/**
 * API callback unlocking an item.
 */
function coregrounds_app_unlock_page() {
  $output['success'] = 0;
  // Validate player session.
  if ($uid = coregrounds_app_validate_session()) {
    $lock = 'unlock_transaction_' . $uid;
    $account = user_load($uid);
    if ($account->data->coins >= 1000) {
      if (lock_acquire($lock)) {
        // Update the player data.
        db_update('coregrounds_players')->fields(array(
          'coins' => ($account->data->coins - 1000),
          'modification_pages' => $account->data->modification_pages + 1,
        ))->condition('uid', $account->uid)->execute();
        // Add the modification page.
        db_insert('coregrounds_modifications')->fields(array(
          'uid' => $account->uid,
          'title' => 'Untitled page',
        ))->execute();
        // Release the lock.
        lock_release($lock);
        // Return success message.
        $output['success'] = 1;
        $output['message'] = 'Unlock successful.';
        $output['player'] = coregrounds_app_build_player_data($uid);
      }
    }
    else {
      $output['message'] = 'Insufficient credits.';
    }
  }
  else {
    $output['message'] = 'Invalid session token';
  }

  coregrounds_app_api_response($output);
}
