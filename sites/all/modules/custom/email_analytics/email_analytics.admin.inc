<?php

/**
 * Admin configuration form.
 */
function email_analytics_admin_config_form($form) {
  $form = array();

  $module_keys = db_select('email_analytics_module_keys', 'k')
    ->fields('k', array('email_module_key'))
    ->execute()
    ->fetchCol();

  $form['email_module_keys'] = array(
    '#type' => 'textarea',
    '#title' => t('Email module keys'),
    '#description' => t('Provide module keys of all emails for which you want to capture analytics. Enter each key id on a new line.'),
    '#default_value' => implode("\n", $module_keys),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );

  return $form;
}

/**
 * Validation callback for admin configuration form.
 */
function email_analytics_admin_config_form_validate($form, &$form_state) {
  $email_module_keys_area = check_plain($form_state['values']['email_module_keys']);
  $email_module_keys = explode("\r", $email_module_keys_area);

  $new_email_module_keys = array();
  foreach ($email_module_keys as $email_module_key) {
    if (!empty($email_module_key)) {
      $new_email_module_keys[] = $email_module_key;
    }
  }

  $form_state['values']['email_module_keys'] = $new_email_module_keys;
}

/**
 * Submit callback for admin configuration form.
 */
function email_analytics_admin_config_form_submit($form, &$form_state) {
  $email_module_keys = $form_state['values']['email_module_keys'];

  $trimmed_email_module_keys = array();
  foreach ($email_module_keys as $email_module_key) {
    $email_module_key = trim($email_module_key);

    db_merge('email_analytics_module_keys')
      ->key(array('email_module_key' => $email_module_key))
      ->execute();

    $trimmed_email_module_keys[] = $email_module_key;
  }

  // Now delete all the keys that are not in email_ids.
  $result = db_delete('email_analytics_module_keys')
    ->condition('email_module_key', $trimmed_email_module_keys, 'NOT IN')
    ->execute();

  drupal_set_message(t('Your changes have been saved.'));
}

/**
 * Returns the default header.
 */
function email_analytics_main_report_default_header() {
  $header = array(
    array(
      'data' => t('Sent'),
      'field' => 'created',
      'sort' => 'desc',
      'weight' => -45,
    ),
    array(
      'data' => t('To'),
      'field' => 'email_to',
      'sort' => 'asc',
      'weight' => -35,
    ),
    array(
      'data' => t('Uid'),
      'key' => 'uid',
      'weight' => -25,
    ),
    array(
      'data' => t('Subject'),
      'key' => 'subject',
      'weight' => -15,
    ),
    array(
      'data' => t('Opened'),
      'field' => 'opened',
      'sort' => 'desc',
      'weight' => -5,
    ),
    array(
      'data' => t('Clicked'),
      'field' => 'clicked',
      'sort' => 'desc',
      'weight' => 5,
    ),
  );

  return $header;
}

/**
 * View email analytics data.
 */
function email_analytics_view() {

  $header = email_analytics_main_report_default_header();
  drupal_alter('email_analytics_main_report_header', $header);

  $query = email_analytics_build_main_report_query($header);
  $result = $query->execute();
  $results = array();
  foreach ($result as $record) {
    $results[] = $record;
  }

  $output = drupal_render(drupal_get_form('email_analytics_main_report_filter_form', $results));
  $output .= theme('email_analytics_main_report', array('result' => $results, 'header' => $header));

  $pager = array('#theme' => 'pager');
  $output .= drupal_render($pager);

  return $output;
}

/**
 * Builds email analytics query for main report.
 */
function email_analytics_build_main_report_query($header = NULL, $limit = NULL) {
  $query = db_select('email_analytics_info', 'eai')
    ->fields('eai',
      array('eid', 'email_to', 'uid', 'email_hash', 'module_key', 'subject', 'created', 'entity_id', 'data'))
    //->orderBy('eai.created', 'DESC')
    ->groupBy('eai.eid');

  if (!empty($_SESSION['email_analytics_main_report']['sent_to'])) {
    $query->condition('eai.email_to', $_SESSION['email_analytics_main_report']['sent_to']);
  }

  if (!empty($_SESSION['email_analytics_main_report']['uid'])) {
    $query->condition('eai.uid', $_SESSION['email_analytics_main_report']['uid']);
  }

  if (!empty($_SESSION['email_analytics_main_report']['min_date'])) {
    $query->condition('eai.created', $_SESSION['email_analytics_main_report']['min_date'], '>=');
  }

  if (!empty($_SESSION['email_analytics_main_report']['max_date'])) {
    $query->condition('eai.created', $_SESSION['email_analytics_main_report']['max_date'], '<=');
  }

  // Extend this query using a pager.
  if (!is_null($limit)) {
    $query = $query->extend('PagerDefault')->limit($limit);
  }

  $open_action_table_query = db_select('email_analytics_data', 'eado')
    ->condition('eado.action', 'open')
    ->groupBy('eado.eid');
  $open_eid_field = $open_action_table_query->addField('eado', 'eid', 'open_eid');
  $open_created_field = $open_action_table_query->addExpression('MIN(eado.created)', 'open_created');

  $click_action_table_query = db_select('email_analytics_data', 'eadc')
    ->condition('eadc.action', 'click')
    ->groupBy('eadc.eid');
  $click_eid_field = $click_action_table_query->addField('eadc', 'eid', 'click_eid');
  $click_created_field = $click_action_table_query->addExpression('MIN(eadc.created)', 'click_created');

  $open_action_table_alias = $query->leftJoin($open_action_table_query, 'eado', 'eai.eid = eado.' . $open_eid_field);
  $click_action_table_alias = $query->leftJoin($click_action_table_query, 'eadc', 'eai.eid = eadc.' . $click_eid_field);

  $main_table_open_field = $query->addField('eai', $open_action_table_alias . '.' . $open_created_field, 'opened');
  $main_table_click_field = $query->addField('eai', $click_action_table_alias . '.' . $click_created_field, 'clicked');

  // Add TableSort extender.
  if (!is_null($header)) {
    $query = $query->extend('TableSort')->orderByHeader($header);
  }

  // Add a tag so that other modules can modify this query.
  $query->addTag('email_analytics_main_report');

  return $query;
}

/**
 * Renders the main report of email analytics data.
 */
function theme_email_analytics_main_report($variables) {

  $result = $variables['result'];
  $header = $variables['header'];

  // If header is not supplied, get the default header.
  if (is_null($header)) {
    $header = email_analytics_main_report_default_header();
    drupal_alter('email_analytics_main_report_header', $header);
  }

  // Iterate through the header and keep only the ones which either don't have any access value or access value is set to TRUE.
  $visible_header = array();
  foreach ($header as $key => $header_cell) {
    if (!isset($header_cell['access'])) {
      $visible_header[] = $header[$key];
    }
    elseif ($header_cell['access']) {
      $visible_header[] = $header[$key];
    }
  }
  uasort($visible_header, 'drupal_sort_weight');

  $rows = array();
  foreach ($result as $record) {
    $row = array();

    // Go through the header and create rows from displayed fields.
    foreach ($visible_header as $header_cell) {
      $field = NULL;
      if (!empty($header_cell['field'])) {
        $field = $header_cell['field'];
      }
      elseif (!empty($header_cell['key'])) {
        $field = $header_cell['key'];
      }

      if (is_null($field)) {
        $row[] = NULL;
        continue;
      }

      if (empty($record->$field)) {
        $row[] = NULL;
        continue;
      }

      $row[] = array(
        'data' => $record->$field,
      );
    }

    $rows[] = $row;
  }

  return theme('table', array(
    'header' => $visible_header,
    'rows' => $rows,
    'sticky' => TRUE,
    'empty' => t('No email analytics data found.'),
  ));
}

/**
 * Creates a filter form for the email analytics main report.
 */
function email_analytics_main_report_filter_form($form, $form_state, $result) {
  $form = array();

  $form['filter'] = array(
    '#type' => 'fieldset',
    '#title' => t('Filter by'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#attached' => array(
      'css' => array(
        drupal_get_path('module', 'email_analytics') . '/css/email_analytics.filters.css',
      ),
    ),
  );

  // Build filters. We don't want to supply $result here since the filters need to be dependent on all the rows of result, not just the ones shown by the pager.
  $form['filter']['filters'] = email_analytics_main_report_filters();

  $form['filter']['result'] = array(
    '#type' => 'value',
    '#value' => $result,
  );

  $form['filter']['actions'] = array(
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Submit'),
      '#name' => 'submit',
    ),
    'reset' => array(
      '#type' => 'submit',
      '#value' => t('Reset'),
      '#name' => 'reset',
    ),
  );

  return $form;
}

/**
 * Submit handler for the filter form.
 */
function email_analytics_main_report_filter_form_submit($form, &$form_state) {
  if ($form_state['triggering_element']['#name'] == 'submit') {
    // Save the values in $_SESSION.
    if (!empty($form_state['values']['sent_to'])) {
      $_SESSION['email_analytics_main_report']['sent_to'] = $form_state['values']['sent_to'];
    }
    else {
      unset($_SESSION['email_analytics_main_report']['sent_to']);
    }

    if (!empty($form_state['values']['uid'])) {
      $_SESSION['email_analytics_main_report']['uid'] = $form_state['values']['uid'];
    }
    else {
      unset($_SESSION['email_analytics_main_report']['uid']);
    }

    if (!empty($form_state['values']['min_date'])) {
      $minDate = new DateTime($form_state['values']['min_date']);
      $_SESSION['email_analytics_main_report']['min_date'] = $minDate->getTimestamp();
    }
    else {
      unset($_SESSION['email_analytics_main_report']['min_date']);
    }

    if (!empty($form_state['values']['max_date'])) {
      $maxDate = new DateTime($form_state['values']['max_date']);
      $_SESSION['email_analytics_main_report']['max_date'] = $maxDate->getTimestamp() + (24 * 60 * 60 - 1);
    }
    else {
      unset($_SESSION['email_analytics_main_report']['max_date']);
    }
  }
  elseif ($form_state['triggering_element']['#name'] == 'reset') {
    unset($_SESSION['email_analytics_main_report']);
  }
}

/**
 * List email analytics administration filters that can be applied.
 */
function email_analytics_main_report_filters($result = NULL) {

  if (is_null($result)) {
    // Find all the possible results.
    $query = email_analytics_build_main_report_query();
    $result = $query->execute();
  }

  // Get email address, uids and date filters from this list.
  $email_addresses = array();
  $uids = array();
  $min_date = NULL;
  $max_date = NULL;
  foreach ($result as $record) {
    if (!is_null($record->uid)) {
      $uids[] = $record->uid;
    }

    if (!in_array($record->email_to, $email_addresses)) {
      $email_addresses[] = $record->email_to;
    }

    $min_date = min($min_date, $record->created);
    $max_date = max($max_date, $record->created);
  }

  $filters = array();

  $filters['sent_to'] = array(
    '#type' => 'select',
    '#title' => t('Email address'),
    '#options' => drupal_map_assoc($email_addresses),
    '#empty_option' => '- None -',
    '#weigth' => -25,
  );
  if (!empty($_SESSION['email_analytics_main_report']['sent_to'])) {
    $filters['sent_to']['#default_value'] = $_SESSION['email_analytics_main_report']['sent_to'];
  }

  $filters['uid'] = array(
    '#type' => 'select',
    '#title' => t('Uid'),
    '#options' => drupal_map_assoc($uids),
    '#empty_option' => '- None -',
    '#weight' => -15,
  );
  if (!empty($_SESSION['email_analytics_main_report']['uid'])) {
    $filters['uid']['#default_value'] = $_SESSION['email_analytics_main_report']['uid'];
  }

  if (module_exists('date_popup')) {
    $filters['min_date'] = array(
      '#type' => 'date_popup',
      '#title' => t('Emails sent after'),
      '#date_type' => DATE_UNIX,
      '#date_format' => 'Y/m/d',
      '#date_increment' => 1,
      '#weight' => -5,
    );
    if (!empty($_SESSION['email_analytics_main_report']['min_date'])) {
      $filters['min_date']['#default_value'] =
        format_date($_SESSION['email_analytics_main_report']['min_date'], 'custom', 'Y-m-d');
    }

    $filters['max_date'] = array(
      '#type' => 'date_popup',
      '#date_type' => DATE_UNIX,
      '#date_increment' => 1,
      '#title' => t('Emails sent before'),
      '#date_format' => 'Y/m/d',
      '#weight' => 5,
    );
    if (!empty($_SESSION['email_analytics_main_report']['max_date'])) {
      $filters['max_date']['#default_value'] =
        format_date($_SESSION['email_analytics_main_report']['max_date'], 'custom', 'Y-m-d');
    }
  }

  return $filters;
}


/**
 * Form to test email analytics.
 */
function email_analytics_test_form($form) {
  $form = array();

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Send test email'),
  );

  return $form;
}


/**
 * Submit handler for email_analytics_test_form.
 */
function email_analytics_test_form_submit() {
  $params = array();
  drupal_mail('email_analytics', 'test', 'neeravbm@yahoo.com', language_default(), $params);
}