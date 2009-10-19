<?php
// $Id$

function services_admin_context_index() {
  return theme('services_context_index', services_get_contexts());
}

function services_admin_context_edit($form_state, $context_name) {
  $context = services_get_context($context_name);
  $form = array();

  $form['path'] = array(
    '#type' => 'textfield',
    '#title' => t('Server endpoint'),
    '#description' => t('This is the endpoint for your server.'),
    '#default_value' => $context['path'],
    '#size' => 40,
    '#maxlength' => 255,
  );

  $auth_modules = module_implements('authentication_info');
  $auth_options = array('' => t('-- Select a authorization module'));
  foreach ($auth_modules as $module) {
    $info = services_auth_info(NULL, $module);
    $auth_options[$info['#description']][$module] = $info['#title'];
  }

  $form['security']['auth_module'] = array(
    '#type' => 'select',
    '#title' => t('Authorization module'),
    '#description' => t('The authorization module determines what authentication scheme should be used with your service. If no authorization module is configured the standard session handling will be used.'),
    '#options' => $auth_options,
    '#required' => FALSE,
    '#default_value' => array($context['authentication']),
    '#ahah' => array(
      'path' => 'admin/services/ahah/security-options',
      'wrapper' => 'security-module-options',
      'method' => 'replace',
    ),
  );

  // Placeholder for the auth module options
  // also used as wrapper for ahah.
  $form['security']['options'] = array(
    '#prefix' => '<div id="security-module-options">',
    '#suffix' => '</div>',
    'settings' => array(
      '#value' => sprintf('<div class="description">%s</div>',
        t('Select a authorization module to configure security')),
    ),
  );
  // Get the configuration form for the authorization module
  if (!empty($context['authentication'])) {
    $settings = services_auth_invoke_custom($context['authentication'], $context['authentication_settings']);
    if (is_array($settings)) {
      $form['security']['options']['settings'] = $settings;
    }
  }

  $resources = services_get_all_resources();
  

  return $form;
}