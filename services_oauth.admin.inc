<?php
// $Id$

function _services_oauth_admin_authorization() {
  $form = array();

  $levels = services_oauth_authorization_levels();
  foreach ($levels as $key => $title) {
    $set = array(
      '#type' => 'fieldset',
      '#title' => $key . ' - ' . $title,
      '#tree' => TRUE,
      'title' => array(
        '#type' => 'textfield',
        '#maxlength' => 255,
        '#title' => t('Title'),
        '#value' => $title,
      ),
    );
    $form[$key] = $set;
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

function _services_oauth_admin_authentication() {
  $form = array();

  $form['intro'] = array('#value' => '<p>' . t('You can change the lowest required OAuth authentication level for resources and methods here. This doesn\'t affect the access checks, so the security of your site <em>should</em> not be affected by changing the authentication requirements.') . '</p>');

  $methods = services_get_all(FALSE);
  $resources = services_get_all_resources(FALSE);
  $auth_levels = array_merge(array('*' => t('Full access')), services_oauth_authorization_levels());

  foreach ($resources as $name => $resource) {
    $ra = array($name => $resource);
    $res_set = array(
      '#type' => 'fieldset',
      '#title' => t('!name resource', array('!name' => $name)),
    );
    $controllers = array();
    services_process_resources($ra, $controllers);
    foreach ($controllers as $path => $controller) {
      $c = array(
        '#type' => 'fieldset',
        '#title' => $path,
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
        '#tree' => TRUE,
      );

      $cred = $controller['#auth'] ? 'token' : ($controller['#key'] ? ($controller['#verify_key'] ? 'consumer' : 'unsigned_consumer') : 'none');
      $c['credentials'] = array(
        '#type' => 'radios',
        '#title' => t('Required authentication'),
        '#options' => array(
          'none' => t('None'),
          'unsigned_consumer' => t('Unsigned with consumer key'),
          'consumer' => t('Consumer key'),
          'token' => t('Consumer key and access token'),
        ),
        '#default_value' => $cred,
      );

      $c['authorization'] = array(
        '#type' => 'checkboxes',
        '#title' => t('Required authorization'),
        '#options' => $auth_levels,
        '#default_value' => $controller['#default_auth_level'] ? $controller['#default_auth_level'] : array('*'),
      );

      $res_set[$path] = $c;
    }

    $form[$name] = $res_set;
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

function _services_oauth_admin_authentication_submit($form, $form_state) {
  $values = $form_state['values'];
  $resources = services_get_all_resources(FALSE);
  $controllers = array();
  $authentication = array();
  services_process_resources($resources, $controllers);
  foreach ($controllers as $path => $controller) {
    $authentication[$path] = $values[$path]['credentials'];
  }
  variable_set('services_oauth_authentication_levels', $authentication);
  
  // Clear the services cache so that methods are updated
  cache_clear_all('services:', 'cache', TRUE);

  drupal_set_message(t('Updated authentication settings'));
}
