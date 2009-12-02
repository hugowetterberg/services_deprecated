<?php
// $Id$

/**
 * @file
 *  The endpoint administration form and page callbacks and their supporting
 *  functions.
 */

function services_admin_endpoint_index() {
  return theme('services_endpoint_index', services_get_endpoints());
}

function services_admin_endpoint_add($form_state) {
  return _services_admin_endpoint_form();
}

function services_admin_endpoint_add_validate($form, $form_state) {
  $values = $form_state['values'];
  if (!preg_match('/^[a-z-]+$/', $values['endpoint'])) {
    form_set_error("endpoint", t("The machine readable name may only contain lower case a-z and dashes."));
  }
}

function services_admin_endpoint_add_submit($form, $form_state) {
  $values = $form_state['values'];
  $values['authentication_settings'] = $values['authentication_settings']['settings'];
  $endpoint = array(
    'resources' => array(),
  );

  foreach (element_children($form) as $name) {
    $info = $form[$name];
    if (isset($info['#endpoint_attribute']) && $info['#endpoint_attribute']) {
      $endpoint[$name] = $values[$name];
    }
  }

  services_write_endpoint($endpoint);
  drupal_set_message('The endpoint has been created', 'info');
  drupal_goto(sprintf('admin/build/services/endpoints/%s/resources', $endpoint['endpoint']));
}

function services_admin_endpoint_edit($form_state, $endpoint_name) {
  return _services_admin_endpoint_form($endpoint_name);
}

function services_admin_endpoint_edit_submit($form, $form_state) {
  $values = $form_state['values'];
  $values['authentication_settings'] = $values['authentication_settings']['settings'];
  $endpoint = services_get_endpoint($values['endpoint']);

  foreach (element_children($form) as $name) {
    $info = $form[$name];
    if (isset($info['#endpoint_attribute']) && $info['#endpoint_attribute']) {
      $endpoint[$name] = $values[$name];
    }
  }

  drupal_set_message('The endpoint has been updated', 'info');
  services_write_endpoint($endpoint);
}

function _services_admin_endpoint_form($endpoint_name=NULL) {
  if (!empty($endpoint_name)) {
    $endpoint = services_get_endpoint($endpoint_name);
  }
  else {
    $endpoint = array();
  }
  $form = array();

  if (!empty($endpoint_name)) {
    $form['endpoint'] = array(
      '#type' => 'value',
      '#value' => $endpoint_name,
    );
  }
  else {
    // TODO: Allow renaming of endpoint that aren't defined in code?
    $form['endpoint'] = array(
      '#type' => 'textfield',
      '#title' => t('Endpoint name'),
      '#default_value' => '',
      '#description' => t('The machine-readable name of your endpoint'),
      '#endpoint_attribute' => TRUE,
      '#required' => TRUE,
    );
  }

  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Endpoint title'),
    '#description' => t('The title for your endpoint.'),
    '#default_value' => $endpoint['title'],
    '#size' => 40,
    '#maxlength' => 255,
    '#required' => TRUE,
    '#endpoint_attribute' => TRUE,
  );

  $servers = services_get_servers();
  $server_opts = array(
    '' => t('-- Select a server'),
  );
  foreach ($servers as $server => $info) {
    $server_opts[$server] = $info['#name'];
  }
  $form['server'] = array(
    '#type' => 'select',
    '#title' => t('Server'),
    '#description' => t('Select a server that should be used in this endpoint'),
    '#options' => $server_opts,
    '#default_value' => $endpoint['server'],
    '#endpoint_attribute' => TRUE,
    '#required' => TRUE,
  );


  $form['path'] = array(
    '#type' => 'textfield',
    '#title' => t('Path to endpoint'),
    '#description' => t('This is the path to your endpoint.'),
    '#default_value' => $endpoint['path'],
    '#size' => 40,
    '#maxlength' => 255,
    '#required' => TRUE,
    '#endpoint_attribute' => TRUE,
  );

  $auth_modules = module_implements('authentication_info');
  $auth_options = array('' => t('-- Select a authorization module'));
  foreach ($auth_modules as $module) {
    $info = services_auth_info(NULL, $module);
    $auth_options[$info['#description']][$module] = $info['#title'];
  }

  $form['authentication'] = array(
    '#type' => 'select',
    '#title' => t('Authentication module'),
    '#description' => t('The authentication module determines what authentication scheme should be used with your endpoint. If no authentication module is configured the standard session handling will be used.'),
    '#options' => $auth_options,
    '#required' => FALSE,
    '#default_value' => array($endpoint['authentication']),
    '#endpoint_attribute' => TRUE,
    '#ahah' => array(
      'path' => 'admin/services/ahah/security-options',
      'wrapper' => 'security-module-options',
      'method' => 'replace',
    ),
  );

  // Placeholder for the auth module options
  // also used as wrapper for ahah.
  $form['authentication_settings'] = array(
    '#prefix' => '<div id="security-module-options">',
    '#suffix' => '</div>',
    '#tree'   => TRUE,
    '#endpoint_attribute' => TRUE,
    'settings' => array(
      '#value' => sprintf('<div class="description">%s</div>',
        // TODO: Make sure that we fall back to using sessions. No 401 plz
        t('Select a authentication module to configure security. endpoints without a configured security module will use standard Drupal sessions - which might be suitable for services consumed by JavaScript and Flash movies on the local site.')),
    ),
  );

  // Get the configuration form for the authorization module
  if (!empty($endpoint['authentication'])) {
    $settings = services_auth_invoke($endpoint['authentication'], 'security_settings', $endpoint['authentication_settings']);
    if (is_array($settings)) {
      $form['authentication_settings']['settings'] = $settings;
    }
  }

  $form['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

function services_admin_endpoint_resources($form_state, $endpoint_name) {
  module_load_include('inc', 'services', 'includes/resource_build');
  $endpoint = services_get_endpoint($endpoint_name);
  $form = array();

  $form['endpoint_name'] = array(
    '#type' => 'value',
    '#value' => $endpoint_name
  );

  $ops = array(
    'create' => t('Create'),
    'retrieve' => t('Retrieve'),
    'update' => t('Update'),
    'delete' => t('Delete'),
    'index' => t('Index'),
  );

  // Call _services_build_resources() directly instead of
  // services_get_resources to bypass caching.
  $resources = _services_build_resources();
  // Apply the endpoint in a non-strict mode, so that the non-active resources
  // are preserved.
  _services_apply_endpoint($resources, $endpoint, FALSE);

  $res = array(
    '#tree' => TRUE,
  );

  foreach ($resources as $name => $resource) {
    $rc = $resource['endpoint'];
    $res_set = array(
      '#type' => 'fieldset',
      '#title' => t('!name resource', array(
        '!name' => preg_replace('/[_-]+/', ' ', $name),
      )),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#tree' => TRUE,
      '#attributes' => array(
        'class' => 'resource',
      ),
    );

    $res_set['alias'] = array(
      '#type' => 'textfield',
      '#title' => t('Alias'),
      '#description' => t('The alias you enter here will be used instead of the resource name.'),
      '#size' => 40,
      '#maxlength' => 255,
      '#default_value' => isset($rc['alias']) ? $rc['alias'] : '',
    );

    $res_set['operations'] = array(
      '#tree' => TRUE,
    );
    foreach ($ops as $op => $title) {
      if (isset($resource[$op])) {
        $res_set['operations'][$op] = array(
          '#type' => 'fieldset',
          '#title' => $title,
          '#collapsible' => TRUE,
          '#collapsed' => TRUE,
        );
        _services_resource_operation_settings($res_set['operations'][$op], $endpoint, $resource, $op);
      }
    }

    $classes = array(
      'actions' => 'actions',
      'targeted_actions' => 'targeted actions',
      'relationships' => 'relationships',
    );
    foreach ($classes as $element => $class) {
      if (!empty($resource[$class])) {
        $res_set[$element] = array(
          '#type' => 'fieldset',
          '#title' => t($class),
          '#tree' => TRUE,
        );
        foreach ($resource[$class] as $action => $definition) {
          $res_set[$element][$action] = array(
            '#type' => 'fieldset',
            '#title' => $action,
            '#collapsible' => TRUE,
            '#collapsed' => TRUE,
          );
          _services_resource_operation_settings($res_set[$element][$action], $endpoint, $resource, $class, $action);
        }
      }
    }

    drupal_alter('services_resource_settings', $res_set, $resource);

    $res[$name] = $res_set;
  }

  $form['resources'] = $res;

  $form['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

function services_admin_endpoint_resources_validate($form, $form_state) {
  $res = $form_state['values']['resources'];

  // Validate aliases
  foreach ($res as $name => $resource) {
    if (!empty($resource['alias'])) {
      if (!preg_match('/^[a-z-]+$/', $resource['alias'])) {
        form_set_error("resources][{$name}][alias", t("The alias for the !name may only contain lower case a-z and dashes.", array(
          '!name' => $form['resources'][$name]['#title'],
        )));
      }
    }
  }
}

function services_admin_endpoint_resources_submit($form, $form_state) {
  $resources = $form_state['values']['resources'];
  $endpoint = services_get_endpoint($form_state['values']['endpoint_name']);

  foreach ($resources as $name => $resource) {
    $c = isset($endpoint['resources'][$name]) ? $endpoint['resources'][$name] : array();

    $c['alias'] = $resource['alias'];
    foreach ($resource['operations'] as $op => $def) {
      $cop = isset($c['operations'][$op]) ? $c['operations'][$op] : array();
      $cop = array_merge($cop, $def);
      $c['operations'][$op] = $cop;
    }

    $classes = array(
      'actions' => 'actions',
      'targeted_actions' => 'targeted actions',
      'relationships' => 'relationships',
    );
    foreach ($classes as $element => $class) {
      if (!empty($resource[$element])) {
        foreach ($resource[$element] as $act => $def) {
          $cop = isset($c[$class][$act]) ? $c[$class][$act] : array();
          $cop = array_merge($cop, $def);
          $c[$class][$act] = $cop;
        }
      }
    }

    $endpoint['resources'][$name] = $c;
  }

  drupal_set_message(t('Your endpoint has been updated'), 'info');
  services_write_endpoint($endpoint);
}

/**
 * Returns information about a resource operation given it's class and name.
 *
 * @return array
 *  Information about the operation, or NULL if no matching
 *  operation was found.
 */
function get_resource_operation_info($resource, $class, $name=NULL) {
  $op = NULL;

  if (isset($resource[$class])) {
    $op = $resource[$class];
    if (!empty($name)) {
      $op = isset($op[$name]) ? $op[$name] : NULL;
    }
  }

  return $op;
}

/**
 * Constructs the settings form for resource operation.
 *
 * @param array $settings
 *  The root element for the settings form.
 * @param string $resource
 *  The resource information array.
 * @param string $class
 *  The class of the operation. Can be 'create', 'retrieve', 'update',
 *  'delete', 'index', 'actions' or 'targeted actions' or 'relationships'.
 * @param string $name
 *  Optional. The name parameter is only used for actions, targeted actions
 *  and relationship.
 */
function _services_resource_operation_settings(&$settings, $endpoint, $resource, $class, $name=NULL) {
  if ($rop = get_resource_operation_info($resource, $class, $name)) {
    $settings['enabled'] = array(
      '#type' => 'checkbox',
      '#title' => t('Enabled'),
      '#default_value' => !empty($rop['endpoint']) && $rop['endpoint']['enabled'],
    );

    if ($rop['endpoint']['preprocess']) {
      $res_set[$op]['preprocess'] = array(
        '#type' => 'item',
        '#title' => t('Preprocess function'),
        '#value' => $rop['endpoint']['preprocess'],
      );
    }

    if ($rop['endpoint']['postprocess']) {
      $res_set[$op]['preprocess'] = array(
        '#type' => 'item',
        '#title' => t('Postprocess function'),
        '#value' => $rop['endpoint']['Postprocess'],
      );
    }

    drupal_alter('services_resource_operation_settings', $settings, $endpoint, $resource, $class, $name);
  }
}

/**
 * Callback for the security configuration form ahah.
 */
function _services_ahah_security_options() {
  $cached_form_state = array();
  $cached_form = form_get_cache($_POST['form_build_id'], $cached_form_state);

  if (!empty($_POST['authentication'])) {
    $settings = services_auth_invoke($_POST['authentication'], 'security_settings');
  }

  if (is_array($settings)) {
    $cached_form['security']['options']['settings'] = $settings;
  }
  else {
    unset($cached_form['security']['options']['settings']);
  }

  form_set_cache($_POST['form_build_id'], $cached_form, $cached_form_state);

  $form_state = array('submitted' => FALSE);
  $options = $cached_form['security']['options'];
  unset($options['#prefix'], $options['#suffix']);
  $options = form_builder('_services_ahah_security_options', $options, $form_state);
  $output = drupal_render($options);

  print drupal_to_js(array(
    'status' => TRUE,
    'data' => $output,
  ));
  exit;
}