<?php
// $Id$

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
  $endpoint = $controller['endpoint'];
  $element['oauth_credentials'] = array(
    '#type' => 'select',
    '#title' => t('Required authentication'),
    '#description' => t('Authorization levels will <em>not</em> be applied if the consumer isn\'t required to supply a access token.'),
    '#options' => array(
      'none' => t('None'),
      'unsigned_consumer' => t('Unsigned with consumer key'),
      'consumer' => t('Consumer key'),
      'token' => t('Consumer key and access token'),
    ),
    '#default_value' => !empty($endpoint['oauth_credentials']) ? $endpoint['oauth_credentials'] : 'token',
  );

  $element['oauth_authorization'] = array(
    '#type' => 'select',
    '#title' => t('Required authorization'),
    '#options' => $auth_levels,
    '#default_value' => !empty($endpoint['oauth_authorization']) ? $endpoint['oauth_authorization'] : '*',
  );
}
