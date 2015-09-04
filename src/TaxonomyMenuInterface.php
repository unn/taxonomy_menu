<?php

/**
 * @file
 * Contains Drupal\taxonomy_menu\TaxonomyMenuInterface.
 */

namespace Drupal\taxonomy_menu;

use Drupal\Core\Config\Entity\ConfigEntityInterface;

/**
 * Provides an interface defining a TaxonomyMenu entity.
 */
interface TaxonomyMenuInterface extends ConfigEntityInterface {
  // Add get/set methods for your configuration properties here.

  /**
   * @return \Drupal\system\Entity\Menu
   */
  public function getMenu();

  /**
   * @return \Drupal\taxonomy\VocabularyInterface
   */
  public function getVocabulary();

  /**
   * Get menu link plugin definitions
   *
   * @param array $base_plugin_definition
   *
   * @param bool $include_base_plugin_id
   *   If true, 'taxonomy_menu.menu_link:' will be prepended to the returned
   *   plugin ids.
   *
   * @return array
   */
  public function getLinks($base_plugin_definition = [], $include_base_plugin_id = FALSE);

}
