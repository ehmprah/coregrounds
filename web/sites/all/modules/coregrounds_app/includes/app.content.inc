<?php

/**
 * @file
 * Provides data for the dashboard.
 */

/**
 * API callback returning the current home page for the app.
 */
function coregrounds_app_home() {
  $output['success'] = 1;

  // Get featured news.
  $q = "SELECT n.nid FROM node n
        INNER JOIN field_data_field_featured f
        ON f.entity_id = n.nid AND f.field_featured_value = 'front'
        WHERE n.type = 'news'
        ORDER BY n.created DESC LIMIT 1";
  if ($nids = db_query($q)->fetchCol()) {
    foreach (node_load_multiple($nids) as $node) {
      $output['data']['news'][] = array(
        'title' => $node->title,
        'url' => url('node/' . $node->nid, array('absolute' => TRUE)),
        'html' => $node->body['und'][0]['value'],
        'image' => image_style_url('480px', $node->field_image['und'][0]['uri']),
      );
    }
  }

  // Get featured video
  $q = "SELECT n.title, y.field_youtube_id_value FROM node n
        INNER JOIN field_data_field_featured f
        ON f.entity_id = n.nid AND f.field_featured_value = 'front'
        INNER JOIN field_data_field_youtube_id y
        ON y.entity_id = n.nid
        WHERE n.type = 'video'
        ORDER BY n.created DESC LIMIT 1";
  if ($video = db_query($q)->fetchObject()) {
    $output['data']['video']['title'] = $video->title;
    $output['data']['video']['id'] = $video->field_youtube_id_value;
  }

  // Get daily free units and abilities.
  $output['data']['free'] = coregrounds_get_free_dailies();

  // Send response.
  coregrounds_app_api_response($output);
}

/**
 * API callback returning the current tos
 */
function coregrounds_app_dev() {
  $node = node_load(255);
  $output['success'] = 1;
  $output['dev'] = $node->body['und'][0]['value'];
  coregrounds_app_api_response($output);
}

/**
 * API callback returning the current home page for the app.
 */
function coregrounds_app_training_grounds() {
  $output['success'] = 1;

  $q = "SELECT y.field_youtube_id_value FROM node n
        INNER JOIN field_data_field_featured f
        ON f.entity_id = n.nid AND f.field_featured_value = 'tg'
        INNER JOIN field_data_field_youtube_id y
        ON y.entity_id = n.nid
        WHERE n.type = 'video'
        ORDER BY n.created DESC";

  $output['data']['videos'] = db_query($q)->fetchCol();

  coregrounds_app_api_response($output);
}
