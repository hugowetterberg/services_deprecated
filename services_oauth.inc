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
  if (!isset($method['#key']) || !$method['#key']) {
    return FALSE;
  }

  try {
    module_load_include('inc', 'oauth_common');

    list($signed, $consumer, $token) = oauth_common_verify_request();

    if (!$signed && $method['#verify_key']) {
      throw new OAuthException('The request must be signed');
    }

    if ($consumer==NULL) {
      throw new OAuthException('Missing consumer token');
    }

    // Validate the token, if it's required by the method
    if ($method['#auth']) {
      if (empty($token->key)) {
        throw new OAuthException('Missing access token');
      }

      if (!$token->authorized) {
        throw new OAuthException('The access token is not authorized');
      }

      // Check that the consumer has been granted the required authorization level
      if (!in_array('*', $token->services) && !in_array($method['#authorization level'], $token->services)) {
        throw new OAuthException('The consumer is not authorized to access this service');
      }
    }

    // Load the user if the request was authenticated using a token
    // that's associated with a account.
    if ($token->uid) {
      global $user;
      $user = user_load($token->uid);
    }
  }
  catch (OAuthException $e) {
    drupal_set_header(sprintf('WWW-Authenticate: OAuth realm="%s"', url('', array('absolute'=>TRUE))));
    return $e->getMessage();
  }
}

function _services_oauth_security_settings($settings) {
  $form = array();
  $form['oauth_context'] = array(
    '#type' => 'select',
    '#title' => t('OAuth context'),
    '#description' => t('The OAuth contexts provides a scope for consumers and authorizations and have their own authorization levels.'),
    '#options' => array(
      '' => t('-- Select a OAuth context'),
      'context_a' => 'Context A',
      'context_b' => 'Context B',
    ),
  );
  return $form;
}

function _services_oauth_security_settings_submit() {
}

/**
 * Alters the methods so that they fit the needs of the OAuth authentication module.
 * This is also where the authentication and authorization levels that are configured
 * in the administration interface are configured.
 *
 * @param array $methods
 * @return void
 */
function _services_oauth_alter_methods(&$methods) {
  $auth = variable_get('services_oauth_authentication_levels', array());
  $authorization = variable_get('services_oauth_authorization_settings', array());
  $autho_levels = oauth_common_authorization_levels();

  foreach ($methods as $key => &$method) {
    // Old-style methods only have a numeric index, take the method
    // name as key instead.
    if (is_numeric($key) && isset($method['#method'])) {
      $key = $method['#method'];
    }

    if (!isset($method[$key]['#auth'])) {
      $method['#auth'] = TRUE;
    }

    if (!isset($method[$key]['#key'])) {
      $method['#key'] = TRUE;
    }

    if (!isset($method['#verify_key'])) {
      $method['#verify_key'] = TRUE;
    }

    // Apply custom authorization level settings
    if (isset($authorization[$key])) {
      if (isset($autho_levels[$authorization[$key]])) {
        $method['#authorization level'] = $authorization[$key];
      }
      else {
        $method['#authorization level'] = '*';
      }
    }
    if (!isset($method['#authorization level'])) {
      $method['#authorization level'] = '*';
    }

    // Check if we got custom settings for the method's authentication
    if (isset($auth[$key])) {
      switch ($auth[$key]) {
        case 'none':
          $method['#verify_key'] = FALSE;
          $method['#key'] = FALSE;
          $method['#auth'] = FALSE;
        break;
        case 'unsigned_consumer':
          $method['#verify_key'] = FALSE;
          $method['#key'] = TRUE;
          $method['#auth'] = FALSE;
        break;
        case 'consumer':
          $method['#verify_key'] = TRUE;
          $method['#key'] = TRUE;
          $method['#auth'] = FALSE;
        break;
        case 'token':
          $method['#verify_key'] = TRUE;
          $method['#key'] = TRUE;
          $method['#auth'] = TRUE;
        break;
      }
    }
  }
}