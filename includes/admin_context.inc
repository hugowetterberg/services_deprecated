<?php
// $Id$

function services_admin_context_index() {
  return theme('services_context_index', services_get_contexts());
}

function services_admin_context_edit($form_state, $context_name) {
  $context = services_get_context($context_name);
  $form = array();

  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Context title'),
    '#description' => t('The title for your context.'),
    '#default_value' => $context['title'],
    '#size' => 40,
    '#maxlength' => 255,
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
    '#description' => t('Select a server that should be used in this context'),
    '#options' => $server_opts,
    '#default_value' => $context['server'],
  );


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
    $settings = services_auth_invoke_custom($context['authentication'], 'security_settings', $context['authentication_settings']);
    if (is_array($settings)) {
      $form['security']['options']['settings'] = $settings;
    }
  }

  return $form;
}

function services_admin_context_resources($form_state, $context_name) {
  $context = services_get_context($context_name);
  $form = array();



  $ops = array(
    'create' => t('Create'),
    'retrieve' => t('Retrieve'),
    'update' => t('Update'),
    'delete' => t('Delete'),
    'index' => t('Index'),
  );

  $resources = services_get_all_resources(TRUE);
  services_apply_context($resources, $context, FALSE);

  foreach ($resources as $name => $resource) {
    $rc = $resource['#context'];
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
      '#default_value' => isset($rc['#alias']) ? $rc['#alias'] : '',
    );

    foreach ($ops as $op => $title) {
      if (isset($resource['#' . $op])) {
        $res_set[$op] = array(
          '#type' => 'fieldset',
          '#title' => $title,
          '#collapsible' => TRUE,
          '#collapsed' => TRUE,
        );
        _services_resource_operation_settings($res_set[$op], $context, $resource, $op);
      }
    }

    drupal_alter('services_resource_settings', $res_set, $resource);

    // $res_set['dmp'] = array(
    //   '#prefix' => '<pre>',
    //   '#suffix' => '</pre>',
    //   '#value' => var_export($resource, TRUE),
    // );

    $form[$name] = $res_set;
  }

  $form['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

function services_admin_context_resources_submit($form, $form_state) {
  var_dump($form_state); die;
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

  if (isset($resource['#' . $class])) {
    $op = $resource['#' . $class];
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
function _services_resource_operation_settings(&$settings, $context, $resource, $class, $name=NULL) {
  if ($rop = get_resource_operation_info($resource, $class, $name)) {
    $settings['enabled'] = array(
      '#type' => 'checkbox',
      '#title' => t('Enabled'),
      '#default_value' => !empty($rop['#context']),
    );

    if ($rop['#context']['preprocess']) {
      $res_set[$op]['preprocess'] = array(
        '#type' => 'item',
        '#title' => t('Preprocess function'),
        '#value' => $rop['#context']['preprocess'],
      );
    }

    if ($rop['#context']['postprocess']) {
      $res_set[$op]['preprocess'] = array(
        '#type' => 'item',
        '#title' => t('Postprocess function'),
        '#value' => $rop['#context']['Postprocess'],
      );
    }

    drupal_alter('services_resource_operation_settings', $settings, $context, $resource, $class, $name);
  }
}