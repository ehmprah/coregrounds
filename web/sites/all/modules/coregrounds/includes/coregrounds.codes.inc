<?php

/**
 * Code Management
 */
function coregrounds_code_management_form($form,&$form_state) {

  $form['generate'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('generate-codes', 'clearfix', 'mv', 'styled')),
    '#tree' => 1,
  );

  $form['generate']['cordium'] = array(
    '#type' => 'textfield',
    '#title' => 'Cordium',
    '#size' => 5,
    '#default_value' => 1000,
  );

  $form['generate']['batch'] = array(
    '#type' => 'textfield',
    '#title' => 'Batch Code',
    '#size' => 10,
    '#maxlength' => 64,
    '#required' => 1,
    '#default_value' => 'PRESS',
  );

  $form['generate']['amount'] = array(
    '#type' => 'select',
    '#title' => 'Amount',
    '#options' => drupal_map_assoc(array(1, 5, 10, 25, 50, 100, 200, 300, 500, 1000)),
    '#required' => 1,
    '#default_value' => 4,
  );

  $form['generate']['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Generate Codes',
    '#submit' => array('coregrounds_generate_codes'),
  );

  $header = array(
    'code' => array('data' => t('Title')),
    'batch' => array('data' => 'Batch', 'field' => 'c.batch','sort' => 'asc'),
    'cordium' => array('data' => 'Cordium', 'field' => 'c.cordium'),
    'used_by' => array('data' => 'Used by', 'field' => 'c.used_by'),
    'sent' => array('data' => 'Sent', 'field' => 'c.public','sort' => 'asc'),
  );

  $options = array();
  $query = db_select('coregrounds_coupons', 'c')->extend('PagerDefault')->extend('TableSort');
  $query->leftjoin('users','u','c.used_by = u.uid');
  $query->fields('u',array('name'));
  if (isset($_GET['show']) && check_plain($_GET['show']) == 'sent') {
    $query->condition('c.public', 1);
  } else {
    $query->condition('c.public', 0);
  }
  $codes = $query->fields('c')->limit(50)->orderByHeader($header)->execute();
  foreach ($codes as $code) {
    $options[$code->code] = array(
      'code' => $code->code,
      'batch' => $code->batch,
      'cordium' => $code->cordium,
      'used_by' => $code->used_by
        ? '<a href="/player/'.$code->used_by.'">' . $code->name . '</a>'
        : $code->used_by,
      'sent' => $code->public,
    );
  }

  $form['actions'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('clearfix','styled')),
  );

  if(isset($_GET['show']) && check_plain($_GET['show']) == 'sent') {
    $form['actions']['show_sent'] = array(
      '#markup' => '<a class="cta" href="/admin/config/coregrounds/codes">Hide sent</a>',
    );
  } else {
    $form['actions']['show_sent'] = array(
      '#markup' => '<a class="cta" href="/admin/config/coregrounds/codes?show=sent">Show sent</a>',
    );
  }

  $form['actions']['delete'] = array(
    '#type' => 'submit',
    '#value' => 'Delete',
    '#submit' => array('coregrounds_delete_codes'),
    '#limit_validation_errors' => array(array('codes')),
    '#attributes' => array('style' => 'float:right;margin:0 0 20px 20px;'),
  );

  $form['actions']['sent'] = array(
    '#type' => 'submit',
    '#value' => 'Set as sent',
    '#submit' => array('coregrounds_send_codes'),
    '#limit_validation_errors' => array(array('codes')),
    '#attributes' => array('style' => 'float:right;margin:0 0 20px 20px;'),
  );

  $form['codes'] = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $options,
    '#empty' => 'No codes available',
  );

  $form['pager'] = array('#markup' => theme('pager'));

  return $form;
}

/**
 * Overview callback
 */
function coregrounds_generate_codes($form,&$form_state) {
  $operations[] = array('coregrounds_generate_codes_batch',array($form_state['values']['generate']));
  batch_set(array(
    'title' => 'Generating codes ...',
    'operations' => $operations,
    'file' => drupal_get_path('module','coregrounds').'/includes/coregrounds.codes.inc',
  ));
  batch_process('admin/config/coregrounds/codes');
}

/**
 * Overview callback
 */
function coregrounds_send_codes($form,&$form_state) {
  if($codes = array_filter($form_state['values']['codes'])) {
    foreach($codes as $code) {
      db_update('coregrounds_coupons')->fields(array(
        'public' => 1,
      ))->condition('code',$code)->execute();
    }
  }
}

/**
 * Overview callback
 */
function coregrounds_delete_codes($form,&$form_state) {
  if($codes = array_filter($form_state['values']['codes'])) {
    foreach($codes as $code) {
      db_delete('coregrounds_coupons')->condition('code',$code)->execute();
    }
  }
}

/**
 * UI Crawler
 */
function coregrounds_generate_codes_batch($generate, &$context) {
  //set persistent variables
  if(!isset($context['sandbox']['codes']['total'])) {
    $context['sandbox']['codes']['generate'] = $generate;
    $context['sandbox']['codes']['total'] = $generate['amount'];
    $context['sandbox']['codes']['done'] = 0;
    $context['sandbox']['codes']['list'] = array();
  }

  //we generate one code per request
  $code = coregrounds_generate_code_string(5);
  //repeat this step until we found one that isn't in the database
  while(db_query("SELECT COUNT(*) FROM coregrounds_coupons WHERE code = '".$code."'")->fetchField()) {
    $code = coregrounds_generate_code_string(5);
  }

  //set fields to write to db
  $fields['code'] = $code;
  $fields['batch'] = $generate['batch'];
  $fields['cordium'] = $generate['cordium'];
  db_insert('coregrounds_coupons')->fields($fields)->execute();

  $context['sandbox']['codes']['list'][] = $code;

  //measure progress
  $context['sandbox']['codes']['done']++;
  $context['finished'] = $context['sandbox']['codes']['done'] / $context['sandbox']['codes']['total'];
  //stop if this was the last one
  if($context['sandbox']['codes']['done'] == $context['sandbox']['codes']['total']) {
    drupal_set_message($context['sandbox']['codes']['total']. " codes generated: <br>" . implode("<br>",$context['sandbox']['codes']['list']));
    $context['finished'] = 1;
  }
}

/**
 * Generate Code
 */
function coregrounds_generate_code_string($length = 8) {
  $characters = '123456789ABCDEFGHIJKLMNPQRSTUVWXYZ';
  $charactersLength = strlen($characters);
  $randomString = '';
  for ($i = 0; $i < $length; $i++) {
    $randomString .= $characters[rand(0, $charactersLength - 1)];
  }
  return $randomString;
}
