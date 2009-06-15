<?php
// $Id: file_service.inc,v 1.1.2.7 2009/02/15 20:33:14 snelson Exp $
/**
 * @author Services Dev Team
 * @file
 *  Link general file functionalities to services module.
 */

/**
 * Get all elements from a given file
 *
 * @param $fid
 *   Number. File ID
 * @return
 *   Array. All elements from a given file
 */
function file_service_get($fid) {
  if ($file = db_fetch_array(db_query('SELECT * FROM {files} WHERE fid = %d', $fid))) {
    $filepath = file_create_path($file['filepath']);
    $binaryfile = fopen($filepath, 'rb');
    $file['file'] = base64_encode(fread($binaryfile, filesize($filepath)));
    fclose($binaryfile);
    return $file;
  }
  else {
    return services_error(t('There is no file with the given ID.'));
  }
}

/**
 * Check if the user has permission to get a given file
 */
function file_service_get_access($fid) {
  global $user;
  if (user_access('get any binary files')) {
    return TRUE;
  }
  elseif ($file = db_fetch_array(db_query('SELECT * FROM {files} WHERE fid = %d', $fid))) {
    return $file['uid'] == $user->uid && user_access('get own binary files');
  }
}

/**
 * Generates an array of base64 encoded files attached to a node
 *
 * @param $nid
 *   Number. Node ID
 * @return
 *   Array. A list of all files from the given node
 */
function file_service_get_node_files($nid) {
  $node = node_load($nid);
  if (isset($node->files)) {
    $files = array();
    foreach ($node->files as $file) {
      // Rebuild the files array so it only contains files we know we're allowed to list
      if ($file->list) {
        $files[] = $file;
      }
    }
    if (count($files) > 0) {
      $send = array();
      foreach ($files as $file) {
        $file = array_shift($files);
        $filepath = file_create_path($file->filepath);
        $binaryfile = fopen($filepath, 'rb');
        $send[$file->fid] = array(
          'file'      => base64_encode(fread($binaryfile, filesize($filepath))),
          'filename'  => $file->filename,
          'uid'       => $file->uid,
          'filemime'  => $file->filemime,
          'filesize'  => $file->filesize,
          'status'    => $file->status,
          'timestamp' => $file->timestamp
        );
        fclose($binaryfile);
      }
    }
    return $send;
  }
  else {
    return services_error(t('There are no files on given node.'));
  }
}

/**
 * Check if the user has permission to get all files from a given node.
 */
function file_service_get_node_files_access($nid) {
  global $user;
  if (user_access('get any binary files')) {
    return TRUE;
  }
  elseif ($node = node_load($nid)) {
    return $node->uid == $user->uid && user_access('get own binary files');
  }
}
