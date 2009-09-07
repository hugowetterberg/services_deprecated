<?php
// $Id: services.resource-translation.inc,v 1.1.2.1 2009/09/05 13:57:58 marcingy Exp $

function _services_resource_as_services($resource) {
  static $controllers = array(
    '#create' => 'create',
    '#delete' => 'delete',
    '#retrieve' => 'retrieve',
    '#update' => 'update',
    '#index' => 'index',
  ), $subcontrollers = array(
    '#relationships' => 'related',
    '#targeted actions' => 'targeted_action',
  );

  $methods = array();
  $file = isset($resource['#file']) ? $resource['#file'] : array();

  foreach ($controllers as $attr => $name) {
    if (isset($resource[$attr])) {
      $methods[] = _services_resource_controller_as_service($resource['#name'], $name, $resource[$attr], $file);
    }
  }

  foreach ($subcontrollers as $attr => $name) {
    if (isset($resource[$attr])) {
      foreach ($resource[$attr] as $sc_name => $controller) {
        $methods[] = _services_resource_controller_as_service($resource['#name'], $name . '_' . $sc_name, $controller, $file);
      }
    }
  }

  if (isset($resource['#actions'])) {
    foreach ($resource['#actions'] as $sc_name => $controller) {
      $methods[] = _services_resource_controller_as_service($resource['#name'], 'action_' . $sc_name, $controller, $file);
    }
  }

  return $methods;
}

function _services_resource_controller_as_service($resource, $name, $controller, $file) {
  $method = array_merge($controller, array(
    '#method' => $resource . '_resource.' . $name,
  ));

  if (!empty($file) && !empty($method['#file'])) {
    $method['#file'] = $file;
  }

  return $method;
}

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