<?php

/**
 * @file
 * This file contains no working PHP code; it exists to document hooks in the
 * standard Drupal manner.
 */

/**
 * Allows modules to perform operations after a menu link resulting from the
 * processing of a taxonomy term has been saved.
 *
 * @param $term
 *   The taxonomy term
 * @param $menu_link
 *   The menu link that has been saved.
 * @param $mlid
 *   The identifier of the newly created menu item.
 */
function hook_taxonomy_menu_save($term, $menu_link, $mlid) {
  // For example, we could process all the translated taxonomy terms of this
  // term here, in order to save their respective translated menu links.
}

/**
 * Allows modules to register new types of path to be used as taxonomy menu links.
 *
 * @return array
 *   An array of types of path, defined by a callback and a label.
 */
function hook_taxonomy_menu_path() {
  // Example from the sub-module "custom paths".
  $paths = array(
    'taxonomy_menu_path_custom' => t('Custom (<base_path>/%)'),
  );

  return $paths;
}
