<?php
// $Id: $

/**
 * @file
 *  The implementation of the session authentication scheme
 */

function _services_sessauth_alter_methods(&$methods) {
  // Skip this if no services have been activated
  if (!is_array($methods) || empty($methods)) {
    return;
  }

  // sessid arg
  $arg_sessid = array(
    'name' => 'sessid',
    'type' => 'string',
    'description' => t('A valid sessid.'),
  );

  foreach ($methods as $key => &$method) {
    // set method defaults
    switch ($method['method']) {
      case 'system.connect':
      case 'search.nodes':
      case 'search.content':
      case 'search.users':
        $method['auth'] = FALSE;
        break;
      default:
        $method['auth'] = TRUE;
        array_unshift($method['args'], $arg_sessid);
    }
  }
}

function _services_sessauth_authenticate_call($method, $method_name, &$args) {
  // Add additonal processing for methods requiring session
  $session_backup = NULL;
  if ($method['auth']) {
    $sessid = array_shift($args);
    if (empty($sessid)) {
      return t('Invalid sessid.');
    }
    $session_backup = services_session_load($sessid);
  }
}

function _services_sessauth_security_settings() {
  return array();
}