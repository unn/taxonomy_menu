<?php

/**
 * @file
 * Contains Drupal\taxonomy_menu\Entity\TaxonomyMenu.
 */

namespace Drupal\taxonomy_menu\Entity;

use Drupal\Core\Config\Entity\ConfigEntityBase;
use Drupal\taxonomy_menu\TaxonomyMenuInterface;

/**
 * Defines the TaxonomyMenu entity.
 *
 * @ConfigEntityType(
 *   id = "taxonomy_menu",
 *   label = @Translation("TaxonomyMenu"),
 *   handlers = {
 *     "list_builder" = "Drupal\taxonomy_menu\Controller\TaxonomyMenuListBuilder",
 *     "form" = {
 *       "add" = "Drupal\taxonomy_menu\Form\TaxonomyMenuForm",
 *       "edit" = "Drupal\taxonomy_menu\Form\TaxonomyMenuForm",
 *       "delete" = "Drupal\taxonomy_menu\Form\TaxonomyMenuDeleteForm"
 *     }
 *   },
 *   config_prefix = "taxonomy_menu",
 *   admin_permission = "administer site configuration",
 *   entity_keys = {
 *     "id" = "id",
 *     "label" = "label",
 *     "uuid" = "uuid"
 *   },
 *   links = {
 *     "edit-form" = "entity.taxonomy_menu.edit_form",
 *     "delete-form" = "entity.taxonomy_menu.delete_form",
 *     "collection" = "entity.taxonomy_menu.collection"
 *   }
 * )
 */
class TaxonomyMenu extends ConfigEntityBase implements TaxonomyMenuInterface {
  /**
   * The TaxonomyMenu ID.
   *
   * @var string
   */
  protected $id;

  /**
   * The TaxonomyMenu label.
   *
   * @var string
   */
  protected $label;

  /**
   * The taxonomy vocabulary.
   *
   * @var \Drupal\taxonomy\Entity\Vocabulary
   */
  protected $vocabulary;

  /**
   * The menu to embed the vocabulary.
   *
   * @var \Drupal\system\Entity\Menu
   */
  protected $menu;

  /**
   * {@inheritdoc}
   */
  public function getVocabulary() {
    return $this->vocabulary;
  }

  /**
   * {@inheritdoc}
   */
  public function getMenu() {
    return $this->menu;
  }

  /**
   * Generate taxonomy menu links.
   *
   * @return array
   */
  public function generateTaxonomyLinks($base_plugin_definition) {

    return \Drupal\taxonomy_menu\Controller\TaxonomyMenu::getTaxonomyMenuLinks($this, $base_plugin_definition);
  }
}
