<?php
// $Id$

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

    if (empty($token->key) && $method['#auth']) {
      throw new OAuthException('Missing access token');
    }

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

function _services_oauth_security_settings() {
  return array();
}

function _services_oauth_security_settings_submit() {
}

function _services_oauth_alter_methods(&$methods) {
  $auth = variable_get('services_oauth_authentication_levels', array());

  foreach ($methods as $key => &$method) {
    if (!isset($method[$key]['#auth'])) {
      $method['#auth'] = TRUE;
    }

    if (!isset($method[$key]['#key'])) {
      $method['#key'] = TRUE;
    }

    if (!isset($method['#verify_key'])) {
      $method['#verify_key'] = TRUE;
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