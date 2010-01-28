<?php
// $Id$

/**
 * Authenticates a call using OAuth to verify the request.
 *
 * @param array $method
 *  The method that's being called
 * @param array $args
 *  The arguments that are being used to call the method
 * @return void|string
 *  Returns nothing, or a error message if authentication fails
 */
function _services_oauth_authenticate_call($method, $args) {
  $endpoint   = $method['endpoint'];
  $cred       = isset($endpoint['oauth_credentials']) ? $endpoint['oauth_credentials'] : 'token';
  $auth_level = isset($endpoint['oauth_authorization']) ? $endpoint['oauth_authorization'] : '*';

  // If no credentials are needed we'll pass this one through
  if ($cred == 'none') {
    return FALSE;
  }

  try {
    module_load_include('inc', 'oauth_common');

    $endpoint_name    = services_get_server_info('endpoint');
    $service_endpoint = services_endpoint_load($endpoint_name);
    list($signed, $consumer, $token) = oauth_common_verify_request();

    if (!$signed && ($cred == 'consumer' || $cred == 'token')) {
      throw new OAuthException('The request must be signed');
    }
    if ($consumer == NULL) {
      throw new OAuthException('Missing consumer token');
    }
    if ($consumer->context !== $service_endpoint->authentication_settings['oauth_context']) {
      throw new OAuthException('The consumer is not valid in the current context');
    }

    // Validate the token, if it's required by the method
    if ($cred == 'token') {
      if (empty($token->key)) {
        throw new OAuthException('Missing access token');
      }
      if (!$token->authorized) {
        throw new OAuthException('The access token is not authorized');
      }
      // Check that the consumer has been granted the required authorization level
      if (!in_array('*', $token->services) && !in_array($auth_level, $token->services)) {
        throw new OAuthException('The consumer is not authorized to access this service');
      }
    }

    // Add the oauth authentication info to server info
    services_set_server_info('oauth_consumer', $consumer);
    services_set_server_info('oauth_token', $token);

    // Load the user if the request was authenticated using a token
    // that's associated with a account.
    if ($token->uid) {
      global $user;
      $user = user_load($token->uid);
    }
  }
  catch (OAuthException $e) {
    drupal_set_header(sprintf('WWW-Authenticate: OAuth realm="%s"', url('', array('absolute' => TRUE))));
    return $e->getMessage();
  }
}

function _services_oauth_security_settings($settings) {
  $form = array();
  $form['oauth_context'] = array(
    '#type'          => 'select',
    '#options'       => array('' => t('-- Select an OAuth context')),
    '#default_value' => isset($settings['oauth_context']) ? $settings['oauth_context'] : '',
    '#title'         => t('OAuth context'),
    '#description'   => t('The OAuth contexts provides a scope for consumers and authorizations and have their own authorization levels. Different services endpoints may share OAuth contexts and thereby allow the use of consumers and tokens across the services endpoint boundraries.'),
  );

  $contexts = oauth_common_authorization_contexts();
  foreach ($contexts as $name => $levels) {
    $form['oauth_context']['#options'][$name] = $name;
  }

  return $form;
}
