<?php
/**
 * @file
 *  Contains functions that only are necessary when a service call is made.
 *  This has broken out so that this code isn't loaded for every page load.
 */

/**
 * Make any changes we might want to make to node.
 */
function services_node_load($node, $fields = array()) {
  if (!isset($node->nid)) {
    return NULL;
  }

  // Loop through and get only requested fields
  if (count($fields) > 0) {
    foreach ($fields as $field) {
      $val->{$field} = $node->{$field};
    }
  }
  else {
    $val = $node;
  }

  return $val;
}

/**
 * Backup current session data and import user session.
 */
function services_session_load($sessid) {
  global $user;

  // If user's session is already loaded, just return current user's data
  if ($user->sid == $sessid) {
    return $user;
  }

  // Make backup of current user and session data
  $backup = $user;
  $backup->session = session_encode();

  // Empty current session data
  $_SESSION = array();

  // Some client/servers, like XMLRPC, do not handle cookies, so imitate it to make sess_read() function try to look for user,
  // instead of just loading anonymous user :).
  if (!isset($_COOKIE[session_name()])) $_COOKIE[session_name()] = $sessid;

  // Load session data
  session_id($sessid);
  sess_read($sessid);

  // Check if it really loaded user and, for additional security, if user was logged from the same IP. If not, then revert automatically.
  if ($user->sid != $sessid) {
    services_session_unload($backup);
    return NULL;
  }

  // Prevent saving of this impersonation in case of unexpected failure.
  session_save_session(FALSE);

  return $backup;
}

/**
 * Revert to previously backuped session.
 */
function services_session_unload($backup) {
  global $user;

  // No point in reverting if it's the same user's data
  if ($user->sid == $backup->sid) {
    return;
  }

  // Some client/servers, like XMLRPC, do not handle cookies, so imitate it to make sess_read() function try to look for user,
  // instead of just loading anonymous user :).
  if (!isset($_COOKIE[session_name()])) $_COOKIE[session_name()] = $backup->sessid;

  // Save current session data
  sess_write($user->sid, session_encode());

  // Empty current session data
  $_SESSION = array();

  // Revert to previous user and session data
  $user = $backup;
  session_id($backup->sessid);
  session_decode($user->session);

  session_save_session(TRUE);
}