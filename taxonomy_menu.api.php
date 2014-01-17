<?php

/**
 * @file
 * This file contains no working PHP code; it exists to document hooks in the
 * standard Drupal manner.
 */

/**
 * Allows modules to alter the menu link before saved.
 *
 * @param $menu_link
 *   The menu link will be saved.
 * @param $term
 *   The taxonomy term
 * @param $menu_name
 *   The machine name of the menu in which the menu link should be saved.
 */
function hook_taxonomy_menu_link_alter(&$menu_link, $term, $menu_name) {
  // For example, change the link_title and options
  $wrapper = entity_metadata_wrapper('taxonomy_term', $term);
  $menu_link['link_title'] .= '<span class="tibetan">' . $wrapper->field_tibetan_name->value() . '</span>';
  $menu_link['options']['html'] = TRUE;
}

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
