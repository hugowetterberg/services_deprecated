<?php
// $Id$

/**
 * @file
 * Administrative functions for the Services module.
 *
 * This provides the UI to list, create, edit and delete endpoints.
 */

/**
 * Output a list of endpoints.
 */
function services_list_endpoint($js = NULL) {
  $header = array(
    array('data' => t('Title'),          'class' => 'services-endpoint-title'),
    array('data' => t('Path'),           'class' => 'services-endpoint-path'),
    array('data' => t('Server'),         'class' => 'services-endpoint-server'),
    array('data' => t('Authentication'), 'class' => 'services-endpoint-authentication'),
    array('data' => t('Storage'),        'class' => 'services-endpoint-storage'),
    array('data' => t('Operations'),     'class' => 'services-endpoint-operations'),
  );

  $endpoints = services_endpoint_load_all();
  $rows = array();

  foreach ($endpoints as $endpoint) {
    $operations = array();

    if (empty($endpoint->disabled)) {
      $operations[] = array(
        'title' => t('Edit'),
        'href'  => 'admin/build/services2/' . $endpoint->endpoint . '/edit',
      );
      $operations[] = array(
        'title' => t('Export'),
        'href'  => 'admin/build/services2/' . $endpoint->endpoint . '/export',
      );
    }

    if ($endpoint->export_type == (EXPORT_IN_CODE | EXPORT_IN_DATABASE)) {
      $operations[] = array(
        'title' => t('Revert'),
        'href'  => 'admin/build/services2/' . $endpoint->endpoint . '/delete',
      );
    }
    elseif ($endpoint->export_type != EXPORT_IN_CODE) {
      $operations[] = array(
        'title' => t('Delete'),
        'href'  => 'admin/build/services2/' . $endpoint->endpoint . '/delete',
      );
    }
    elseif (empty($endpoint->disabled)) {
      $operations[] = array(
        'title' => t('Disable'),
        'href'  => 'admin/build/services2/' . $endpoint->endpoint . '/disable',
        'query' => drupal_get_destination(),
      );
    }
    else {
      $operations[] = array(
        'title' => t('Enable'),
        'href'  => 'admin/build/services2/' . $endpoint->endpoint . '/enable',
        'query' => drupal_get_destination(),
      );
    }

    $rows[$endpoint->endpoint] = array(
      'data' => array(
        'title' => array(
          'data'  => $endpoint->title,
          'class' => 'services-endpoint-title',
        ),
        'path' => array(
          'data'  => l($endpoint->path, $endpoint->path),
          'class' => 'services-endpoint-path',
        ),
        'server' => array(
          'data'  => check_plain($endpoint->server),
          'class' => 'services-endpoint-server',
        ),
        'authentication' => array(
          'data'  => check_plain($endpoint->authentication),
          'class' => 'services-endpoint-authentication',
        ),
        'storage' => array(
          'data'  => ($preset->export_type == EXPORT_IN_CODE) ? t('In code') : t('In database'),
          'class' => 'services-endpoint-storage',
        ),
        'operations' => array(
          'data'  => theme('links', $operations),
          'class' => 'services-endpoint-operations',
        ),
      ),
      'class' => 'services-endpoint-' . $endpoint->endpoint . (!empty($endpoint->disabled) ? ' services-endpoint-disabled' : ''),
    );
  }

  $table = theme('table', $header, $rows, array('id' => 'services-list-endpoint'));

  drupal_add_css(drupal_get_path('module', 'services') . '/css/services.admin.css');

  return $table;
}

/**
 * Handle the add endpoint page..
 */
function services_add_endpoint() {
  $endpoint = services_endpoint_new();
  drupal_set_title(t('Add endpoint'));
  return services_edit_endpoint($endpoint);
}

/**
 * Edit an endpoint.
 *
 * Called from both the add and edit points to provide for common flow.
 */
function services_edit_endpoint($endpoint) {
  if (!is_object($endpoint)) {
    $endpoint = services_endpoint_load($endpoint);
  }
  if ($endpoint && !empty($endpoint->title)) {
    drupal_set_title(check_plain($endpoint->title));
  }
  return drupal_get_form('services_edit_form_endpoint', $endpoint);
}

/**
 * Form to edit the settings of an endpoint.
 */
function services_edit_form_endpoint(&$form_state, $endpoint) {
  $form = array();

  $form['eid'] = array(
    '#type'  => 'value',
    '#value' => isset($endpoint->eid) ? $endpoint->eid : '',
  );

  $form['endpoint_object'] = array(
    '#type'  => 'value',
    '#value' => $endpoint,
  );

  $form['endpoint'] = array(
    '#type'          => 'textfield',
    '#size'          => 24,
    '#maxlength'     => 255,
    '#default_value' => $endpoint->endpoint,
    '#title'         => t('Endpoint name'),
    '#description'   => t('A unique name used to identify this preset internally. It must be only be alpha characters and underscores. No spaces, numbers or uppercase characters.'),
    '#required'      => TRUE,
  );

  $form['title'] = array(
    '#type'          => 'textfield',
    '#size'          => 24,
    '#maxlength'     => 255,
    '#default_value' => $endpoint->title,
    '#title'         => t('Endpoint title'),
    '#required'      => TRUE,
  );

  $servers = services_get_servers();
  $server_opts = array(
    '' => t('-- Select a server'),
  );
  foreach ($servers as $server => $info) {
    $server_opts[$server] = $info['#name'];
  }
  $form['server'] = array(
    '#type'          => 'select',
    '#options'       => $server_opts,
    '#default_value' => $endpoint->server,
    '#title'         => t('Server'),
    '#description'   => t('Select a server that should be used in this endpoint'),
    '#required'      => TRUE,
  );

  $label = empty($endpoint->eid) ? t('Save and proceed') : t('Save');
  $form['submit'] = array(
    '#type'  => 'submit',
    '#value' => $label,
  );

  return $form;
}