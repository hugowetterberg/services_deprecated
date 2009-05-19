<?php
// $Id$

/**
 * Menu system callback that generates a form for
 * administrating the authorization levels.
 *
 * @return array
 *  A form array.
 */
function _services_oauth_admin_authorization() {
  $form = array();

  $levels = services_oauth_authorization_levels();
  foreach ($levels as $name => $level) {
    $set = array(
      '#type' => 'fieldset',
      '#title' => $name . ' - ' . $level->title,
      '#tree' => TRUE,
      'title' => array(
        '#type' => 'textfield',
        '#maxlength' => 100,
        '#title' => t('Title'),
        '#default_value' => $level->title,
      ),
      'description' => array(
        '#type' => 'textarea',
        '#maxlength' => 255,
        '#title' => t('Description'),
        '#default_value' => $level->description,
      ),
      'delete' => array(
        '#type' => 'item',
        '#value' => l('Delete', 'admin/build/services/authorization/' . $name . '/delete'),
      ),
    );
    $form[$name] = $set;
  }

  $form['add_level'] = array(
    '#type' => 'fieldset',
    '#tree' => TRUE,
    '#title' => t('Add a authorization level'),
    'name' => array(
      '#type' => 'textfield',
      '#maxlength' => 32,
      '#title' => t('Name'),
      '#default_value' => '',
    ),
    'title' => array(
      '#type' => 'textfield',
      '#maxlength' => 100,
      '#title' => t('Title'),
      '#default_value' => '',
    ),
    'description' => array(
      '#type' => 'textarea',
      '#maxlength' => 255,
      '#title' => t('Description'),
      '#default_value' => '',
    ),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

function _services_oauth_admin_authorization_submit($form, $form_state) {
  $values = $form_state['values'];
  $levels = services_oauth_authorization_levels();

  // Update titles and descriptions
  foreach ($levels as $name => $level) {
    services_oauth_write_authorization_level($name, $values[$name]['title'], $values[$name]['description']);
  }

  // Add a authorization level if the name and title fields have been filled
  $add = $values['add_level'];
  if (!empty($add['name']) && !empty($add['title'])) {
    services_oauth_write_authorization_level($add['name'], $add['title'], $add['description']);
  }

  // Clear the services cache so that methods are updated
  cache_clear_all('services:', 'cache', TRUE);
}

/**
 * Menu system callback for the confirmation page when deleting a authorization
 * level.
 *
 * @param array $form_state
 * @param string $authorization
 *  The machine-readable name of the authorization level.
 * @return array
 *  A form array.
 */
function _services_oauth_admin_authorization_delete($form_state, $authorization) {
  $levels = services_oauth_authorization_levels();

  drupal_set_title(t('Deleting "!title"', array(
    '!title' => $levels[$authorization]->title,
  )));

  $form = array(
    'authorization' => array(
      '#type' => 'value',
      '#value' => $authorization,
    ),
  );

  $form['description'] = array(
    '#type' => 'item',
    '#value' => t('Are you sure that you want to delete the authorization level "!title" (!name). Operations that have been set to require this authorization level will be set to require full access. This could be a <em>very bad thing</em> on a production site, and <em>will</em> break applications that integrate with your site (if they have been granted this authorization level and depend on it). Only do this if you <em>really</em> know what you\'re doing.', array(
      '!title' => $levels[$authorization]->title,
      '!name' => $authorization,
    )),
  );

  $form['confirm'] = array(
    '#type' => 'checkbox',
    '#title' => t('I\'ve really read and understood the above warning. Delete the authorization level please!'),
    '#required' => TRUE,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Delete'),
  );

  return $form;
}

function _services_oauth_admin_authorization_delete_submit($form, $form_state) {
  $authorization = $form_state['values']['authorization'];
  $levels = services_oauth_authorization_levels();
  services_oauth_delete_authorization_level($authorization);
  drupal_set_message(t('The authorization level "!title" has been deleted', array('!title' => $levels[$authorization]->title)));
  drupal_goto('admin/build/services/authorization');
}

/**
 * Menu system callback that generates a form for the authentication and
 * authorization settings for individual resource controllers and methods.
 *
 * @return array
 *  A form array.
 */
function _services_oauth_admin_authentication() {
  $form = array();

  drupal_add_css(drupal_get_path('module', 'services_oauth') . '/services_oauth.css');

  $form['intro'] = array('#value' => '<p>' . t('You can change the lowest required OAuth authentication level and the authorization level for resources and methods here. This doesn\'t affect the access checks, so the security of your site <em>should</em> not be affected by changing the authentication requirements.') . '</p>');

  $auth_levels = array('*' => t('Full access'));
  foreach (services_oauth_authorization_levels() as $name => $level) {
    $auth_levels[$name] = t($level->title);
  }

  // Display resources
  // This part of the module will not activate until the
  // resource patch gets accepted into services
  if (function_exists('services_get_all_resources') && function_exists('services_process_resources')) {
    $resources = services_get_all_resources(FALSE);
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
  $methods = services_get_all(FALSE);
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
    '#type' => 'radios',
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
    '#type' => 'radios',
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
    $resources = services_get_all_resources(FALSE);
    $controllers = array();
    services_process_resources($resources, $controllers);
    foreach ($controllers as $path => $controller) {
      $authentication[$path] = $values[$path]['credentials'];
      $authorization[$path] = $values[$path]['authorization'];
    }
  }

  // Process all methods
  $methods = services_get_all(FALSE);
  foreach ($methods as $method) {
    $path = str_replace('.', '_', $method['#method']);
    $authentication[$method['#method']] = $values[$path]['credentials'];
    $authorization[$method['#method']] = $values[$path]['authorization'];
  }

  variable_set('services_oauth_authentication_levels', $authentication);
  variable_set('services_oauth_authorization_settings', $authorization);

  // Clear the services cache so that methods are updated
  cache_clear_all('services:', 'cache', TRUE);

  drupal_set_message(t('Updated authentication settings'));
}
