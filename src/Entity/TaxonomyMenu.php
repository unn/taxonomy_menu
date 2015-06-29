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
    $links = [];

    // Load taxonomy terms for tax menu vocab.
    $terms = \Drupal::entityManager()->getStorage('taxonomy_term')->loadTree($this->getVocabulary());

    $taxonomy_menu_id = $this->id();

    // Create menu links for each term in the vocabulary.
    foreach ($terms as $term_data) {
      // Load the actual term entity for full info (does not contain parents).
      $term = \Drupal\taxonomy\Entity\Term::load($term_data->tid);

      $term_id = $term->id();
      $term_url = $term->urlInfo();

      // Uniquely identify this menu link.
      $menu_link_id = 'taxonomy_menu.menu_link.' . $taxonomy_menu_id . '.' . $term_id;

      // Determine parent link.
      // TODO: Evaluate use case of multiple parents (should we make many menu items?)
      $menu_parent_id = NULL;
      if (is_array($term_data->parents) and $term_data->parents[0] != '0') {
        $menu_parent_id = 'taxonomy_menu.menu_link.' . $taxonomy_menu_id . '.' . $term_data->parents[0];
      }

      // TODO: Consider implementing a forced weight based on taxonomy tree.

      // Generate link.
      //$arguments = $term_url->getRouteParameters() + ['taxonomy_term' => $term_id];
      $arguments = ['taxonomy_term' => $term_id];
      $links[$menu_link_id] = array(
        'id' => $menu_link_id,
        'title' => $term->label(),
        'description' => $term->getDescription(),
        'menu_name' => $this->getMenu(),
        'metadata' => array(
          'taxonomy_menu_id' => $taxonomy_menu_id,
        ),
        'route_name' => $term_url->getRouteName(),
        'route_parameters' => $term_url->getRouteParameters(),
        'load arguments'  => $arguments,
        'parent' => $menu_parent_id,
      );

      // KRIS - ADDING THIS LINE THROWS ERROR
      $links[$menu_link_id] = $links[$menu_link_id] + $base_plugin_definition;

      var_dump($links[$menu_link_id]);
    }

    return $links;
  }
}
