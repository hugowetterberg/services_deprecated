<?php
// $Id$

/**
 * Menu system callback that generates a form for the authentication and
 * authorization settings for individual resource controllers and methods.
 *
 * @return array
 *  A form array.
 */
function _services_oauth_admin_authentication($form_state, $context='') {
  $form = array(
    'context' => array(
      '#type' => 'value',
      '#value' => $context,
    ),
  );

  drupal_add_css(drupal_get_path('module', 'services_oauth') . '/services_oauth.css');

  $form['intro'] = array('#value' => '<p>' . t('You can change the lowest required OAuth authentication level and the authorization level for resources and methods here. This doesn\'t affect the access checks, so the security of your site <em>should</em> not be affected by changing the authentication requirements.') . '</p>');

  $auth_levels = array('*' => t('Full access'));
  foreach (oauth_common_authorization_levels($context) as $name => $level) {
    $auth_levels[$name] = t($level->title);
  }

  // Display resources
  // This part of the module will not activate until the
  // resource patch gets accepted into services
  if (function_exists('services_get_all_resources') && function_exists('services_process_resources')) {
    $resources = services_get_all_resources(FALSE, $context);
    foreach ($resources as $name => $resource) {
      $ra = array($name => $resource);
      $res_set = array(
        '#type' => 'fieldset',
        '#title' => t('!name resource', array('!name' => $name)),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      );
      $controllers = array();
      services_process_resources($ra, $controllers);
      foreach ($controllers as $path => $controller) {
        list($res, $con) = preg_split('/\//', $path, 2);
        $c = array(
          '#type' => 'fieldset',
          '#title' => $con,
          '#collapsible' => TRUE,
          '#collapsed' => TRUE,
          '#tree' => TRUE,
          '#attributes' => array(
            'class' => 'auth-authorization',
          ),
        );

        _services_oauth_admin_authentication_options($c, $controller, $auth_levels);

        $res_set[$path] = $c;
      }
      $form[$name] = $res_set;
    }
  }

  // Display methods
  $methods = services_get_all(FALSE, $context);
  foreach ($methods as $method) {
    $path = str_replace('.', '_', $method['#method']);
    $m = array(
      '#type' => 'fieldset',
      '#title' => $method['#method'],
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#tree' => TRUE,
      '#attributes' => array(
        'class' => 'auth-authorization',
      ),
    );
    _services_oauth_admin_authentication_options($m, $method, $auth_levels);
    $form[$path] = $m;
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Helper function for generating authentication and authorization options for
 * both controllers and methods.
 *
 * @param array $element
 *  The form element that the options should be added to.
 * @param array $controller
 *  The controller or method that's being configured
 * @param array $auth_levels
 *  The available authorization levels
 * @return void
 */
function _services_oauth_admin_authentication_options(&$element, $controller, $auth_levels) {
  $cred = $controller['#auth'] ? 'token' : ($controller['#key'] ? ($controller['#verify_key'] ? 'consumer' : 'unsigned_consumer') : 'none');
  $element['credentials'] = array(
    '#type' => 'select',
    '#title' => t('Required authentication'),
    '#description' => t('Authorization levels will <em>not</em> be applied if the consumer isn\'t required to supply a access token.'),
    '#options' => array(
      'none' => t('None'),
      'unsigned_consumer' => t('Unsigned with consumer key'),
      'consumer' => t('Consumer key'),
      'token' => t('Consumer key and access token'),
    ),
    '#default_value' => $cred,
  );

  $element['authorization'] = array(
    '#type' => 'select',
    '#title' => t('Required authorization'),
    '#options' => $auth_levels,
    '#default_value' => $controller['#authorization level'],
  );
}

function _services_oauth_admin_authentication_submit($form, $form_state) {
  $values = $form_state['values'];
  $authentication = array();
  $authorization = array();

  // Process all resources
  if (function_exists('services_get_all_resources') && function_exists('services_process_resources')) {
    $resources = services_get_all_resources(FALSE, $values['context']);
    $controllers = array();
    services_process_resources($resources, $controllers, $values['context']);
    foreach ($controllers as $path => $controller) {
      $authentication[$path] = $values[$path]['credentials'];
      $authorization[$path] = $values[$path]['authorization'];
    }
  }

  // Process all methods
  $methods = services_get_all(FALSE, $values['context']);
  foreach ($methods as $method) {
    $path = str_replace('.', '_', $method['#method']);
    $authentication[$method['#method']] = $values[$path]['credentials'];
    $authorization[$method['#method']] = $values[$path]['authorization'];
  }

  variable_set('services_oauth_authentication_levels_' . $values['context'], $authentication);
  variable_set('services_oauth_authorization_settings_' . $values['context'], $authorization);

  // Clear the services cache so that methods are updated
  cache_clear_all('services:', 'cache', TRUE);

  drupal_set_message(t('Updated authentication settings'));
}
