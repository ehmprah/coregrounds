<?php

/**
 * @file
 * Delivers the current configuration.
 */

/**
 * The delivery callback.
 */
function coregrounds_api_deliver_config() {
  $verified = FALSE;
  // Check if we have a valid API key.
  // TODO remove this once the lobby server handles this via POST.
  if (isset($_GET['key']) && $_GET['key'] === COREGROUNDS_APP_KEY) {
    $verified = TRUE;
  }
  if (isset($_POST['key']) && $_POST['key'] === COREGROUNDS_APP_KEY) {
    $verified = TRUE;
  }
  if ($verified) {
    $file = drupal_get_path('module', 'coregrounds_app') . '/server_config.json';
    print file_get_contents($file);
  }
  else {
    $output['message'] = 'Invalid API key.';
    drupal_json_output($output);
  }
  drupal_exit();
}
