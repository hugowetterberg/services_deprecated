<?php
// $Id: services.resource-translation.inc,v 1.1.2.1 2009/09/05 13:57:58 marcingy Exp $

function _services_services_as_resources($services) {
  $resources = array();

  foreach ($services as $service) {
    $signature = preg_split('/\./', $service['#method']);

    $controller = $service;
    $controller['#args'] = array();

    foreach($service['#args'] as $arg) {
      $arg['#source'] = array(
        'data' => $arg['#name'],
      );
      $controller['#args'][] = $arg;
    }

    $resources['service_' . $signature[0]]['#actions'][$signature[1]] = $controller;
  }
  return $resources;
}