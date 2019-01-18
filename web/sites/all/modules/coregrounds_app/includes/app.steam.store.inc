<?php

define("SANDBOX", false);
if (SANDBOX == true) {
  define('STEAM_ENDPOINT', 'https://partner.steam-api.com/ISteamMicroTxnSandbox');
} else {
  define('STEAM_ENDPOINT', 'https://partner.steam-api.com/ISteamMicroTxn');
}

/**
 * API callback returning country and currency code for the user
 */
function coregrounds_app_steam_store_get_user_info() {
  $output['success'] = 0;
  $url = STEAM_ENDPOINT . '/GetUserInfo/v2/';
  $url .= '?key=' . STEAM_API_KEY;
  $url .= '&steamid=' . check_plain($_POST['steamid']);
  if ($response = file_get_contents($url)) {
    // watchdog('coregrounds', 'GetUserInfo: ' .print_r($response,1));
    if ($parsed = json_decode($response, TRUE)) {
      if ($parsed['response']['result'] == 'OK') {
        $output['success'] = 1;
        $output['params'] = $parsed['response']['params'];
      }
    }
  }
  coregrounds_app_api_response($output);
}

/**
 * API callback initializing a microtransaction
 */
function coregrounds_app_steam_store_transaction_init() {
  $output['success'] = 0;
  // Validate player session.
  if ($uid = coregrounds_app_validate_session()) {
    $steamid = _get_steamid_from_uid($uid);
    if ($bundle = configQuery('purchases', 'id', $_POST['bundle'])) {
      // Try to get price for currency from bundle, with USD as fallback
      if (isset($bundle['prices'][$_POST['currency']])) {
        $amount = $bundle['prices'][$_POST['currency']];
        $currency = $_POST['currency'];
      } else {
        $amount = $bundle['prices']['USD'];
        $currency = 'USD';
      }
      // Create the transaction in the database
      $orderid = db_insert('coregrounds_transactions')->fields([
        'uid' => $uid,
        'bundle' => $_POST['bundle'],
        'created' => REQUEST_TIME,
        'changed' => REQUEST_TIME,
        'country' => $_POST['country'],
        'currency' => $currency,
        'status' => 'initialized',
      ])->execute();

      // Build fields to send to steam api
      $fields = [
        'key' => STEAM_API_KEY,
        'orderid' => $orderid,
        'steamid' => $steamid,
        'appid' => 649770,
        'itemcount' => 1,
        'language' => _get_web_api_language_code($_POST['language']),
        'currency' => $currency,
        'itemid[0]' => $_POST['bundle'],
        'qty[0]' => 1,
        'amount[0]' => $amount,
        'description[0]' => $bundle['name'],
      ];

      // Specific handling for the web-based purchasing
      if (isset($_POST['web'])) {
        $fields['usersession'] = 'web';
        $fields['ipaddress'] = ip_address();
      }

      // Build API request to the steam servers
      $context  = stream_context_create(array(
        'http' => array(
          'header'  => "Content-type: application/x-www-form-urlencoded",
          'method'  => 'POST',
          'content' => http_build_query($fields),
        ),
      ));
      // Send request and process response
      if ($response = file_get_contents(STEAM_ENDPOINT . '/InitTxn/v3/', false, $context)) {
        // watchdog('coregrounds', 'InitTxn: ' .print_r($response,1));
        if ($parsed = json_decode($response, TRUE)) {
          if ($parsed['response']['result'] == 'OK') {
            $output['success'] = 1;
            if (isset($parsed['response']['params']['steamurl'])) {
              $output['orderid'] = $orderid;
              $output['steamurl'] = $parsed['response']['params']['steamurl'];
            }
            db_update('coregrounds_transactions')->fields([
              'txnid' => $parsed['response']['params']['transid'],
            ])->condition('orderid', $orderid)->execute();
          }
        }
      }
    }
  }
  coregrounds_app_api_response($output);
}

/**
 * API callback finalizing a microtransaction
 */
function coregrounds_app_steam_store_transaction_finalize() {
  $output['success'] = 0;
  // Validate player session.
  if ($uid = coregrounds_app_validate_session()) {
    $order = _load_microtxn($_POST['orderid']);
    $bundle = configQuery('purchases', 'id', $order->bundle);
    // Make sure we don't have a status yet so players can't unlock the same order multiple times
    if ($order->status === 'initialized') {
      // Build API request to the steam servers
      $context  = stream_context_create(array(
        'http' => array(
          'header'  => "Content-type: application/x-www-form-urlencoded",
          'method'  => 'POST',
          'content' => http_build_query([
            'key' => STEAM_API_KEY,
            'orderid' => $order->orderid,
            'appid' => 649770,
          ]),
        ),
      ));
      // Send request and process response
      if ($response = file_get_contents(STEAM_ENDPOINT . '/FinalizeTxn/v2/', false, $context)) {
        // watchdog('coregrounds', 'FinalizeTxn: ' .print_r($response,1));
        if ($parsed = json_decode($response, TRUE)) {
          if ($parsed['response']['result'] == 'OK') {
            finalize_order($order);
            $account = user_load($uid);
            coregrounds_update_achievements($account, 1, [205]);
            $output['success'] = 1;
            $output['currency'] = $account->data->pc_current + $bundle['currency'];
            $output['achievements'] = coregrounds_get_unread_achievements($account->uid);
          }
        }
      }
    }
  }
  coregrounds_app_api_response($output);
}

/**
 * Web callback finalizing a microtransaction
 */
function coregrounds_app_steam_store_transaction_finalize_web($id) {
  // Load transaction
  $q = "SELECT * FROM coregrounds_transactions WHERE orderid = :id AND status = :status";
  if ($order = db_query($q, [':id' => $id, ':status' => 'initialized'])->fetchObject()) {
    // Build query url
    $url = STEAM_ENDPOINT . '/QueryTxn/v2/?' . http_build_query([
      'key' => STEAM_API_KEY,
      'appid' => 649770,
      'orderid' => $order->orderid,
      'transid' => $order->txnid,
    ]);
    // Send request and process response
    if ($response = file_get_contents($url)) {
      // watchdog('coregrounds', 'WebFinalize: ' .print_r($response,1));
      $parsed = json_decode($response, TRUE);
      if ($parsed && $parsed['response']['result'] == 'OK') {
        if ($parsed['response']['params']['status'] === 'Approved') {
          // Build API request to the steam servers
          $context  = stream_context_create(array(
            'http' => array(
              'header'  => "Content-type: application/x-www-form-urlencoded",
              'method'  => 'POST',
              'content' => http_build_query([
                'key' => STEAM_API_KEY,
                'orderid' => $order->orderid,
                'appid' => 649770,
              ]),
            ),
          ));
          // Send request and process response
          if ($response = file_get_contents(STEAM_ENDPOINT . '/FinalizeTxn/v2/', false, $context)) {
            // watchdog('coregrounds', 'FinalizeTxn: ' .print_r($response,1));
            if ($parsed = json_decode($response, TRUE)) {
              if ($parsed['response']['result'] == 'OK') {
                finalize_order($order);
                return array(
                  '#markup' => '<h1>Transaction successful</h1><p>Thank you for your purchase!</p>',
                );
              }
            }
          }
        }
      }
    }
  }
  return MENU_NOT_FOUND;
}

/**
 * API callback cancelling a microtransaction
 */
function coregrounds_app_steam_store_transaction_cancel() {
  $output['success'] = 0;
  // Validate player session.
  if ($uid = coregrounds_app_validate_session()) {
    $order = _load_microtxn($_POST['orderid']);
    // Update the transaction with a temporary status
    db_update('coregrounds_transactions')->fields([
      'status' => 'cancelled',
    ])->condition('orderid', $order->orderid)->execute();
    $output['success'] = 1;
  }
  coregrounds_app_api_response($output);
}

/**
 * Helper function loading the steam id for a given user id
 */
function _get_steamid_from_uid($uid) {
  $q = "SELECT steamid FROM coregrounds_players WHERE uid = :uid";
  return db_query($q, [':uid' => $uid])->fetchField();
}

/**
 * Helper function loading a microtransaction object
 */
function _load_microtxn($orderid) {
  $q = "SELECT * FROM coregrounds_transactions WHERE orderid = :orderid";
  return db_query($q, [':orderid' => $orderid])->fetchObject();
}

/**
 * Helper function translating game api lang code to web api lang code
 * SEE https://partner.steamgames.com/doc/store/localization#supported_languages
 */
function _get_web_api_language_code($lang) {
  $langs = [
    'arabic' => 'ar',
    'bulgarian' => 'bg',
    'schinese' => 'zh-CN',
    'tchinese' => 'zh-TW',
    'czech' => 'cs',
    'danish' => 'da',
    'dutch' => 'nl',
    'english' => 'en',
    'finnish' => 'fi',
    'french' => 'fr',
    'german' => 'de',
    'greek' => 'el',
    'hungarian' => 'hu',
    'italian' => 'it',
    'japanese' => 'ja',
    'koreana' => 'ko',
    'norwegian' => 'no',
    'polish' => 'pl',
    'portuguese' => 'pt',
    'brazilian' => 'pt-BR',
    'romanian' => 'ro',
    'russian' => 'ru',
    'spanish' => 'es',
    'swedish' => 'sv',
    'thai' => 'th',
    'turkish' => 'tr',
    'ukrainian' => 'uk',
  ];
  return $langs[$lang];
}

/**
 * Helper function finalizing an order
 */
function finalize_order($order) {
  $lock = 'txn_' . $order->orderid;
  $account = user_load($order->uid);
  $bundle = configQuery('purchases', 'id', $order->bundle);
  if (lock_acquire($lock)) {
    // Unlock cryptos
    db_update('coregrounds_players')->fields([
      'pc_current' => $account->data->pc_current + $bundle['currency'],
      'pc_bought' => $account->data->pc_bought + $bundle['currency'],
    ])->condition('uid', $account->uid)->execute();
    // Update the transaction with a temporary status
    db_update('coregrounds_transactions')->fields([
      'status' => 'finalized',
      'changed' => REQUEST_TIME,
    ])->condition('orderid', $order->orderid)->execute();
    // Release the lock.
    lock_release($lock);
    // Unlock stuff included in the bundle
    if (isset($bundle['unlocks']) && !empty($bundle['unlocks'])) {
      $query = db_insert('coregrounds_unlocks')->fields(['uid', 'unlid']);
      foreach ($bundle['unlocks'] as $id) {
        $query->values([
          'uid' => $account->uid,
          'unlid' => $id,
        ]);
      }
      $query->execute();
    }
  }
}
