<?php

/**
 * @file
 * Contains Drupal\taxonomy_menu\Controller\TaxonomyMenu.
 */

namespace Drupal\taxonomy_menu\Controller;

use Drupal\Core\Controller\ControllerBase;
use Drupal\Core\Menu\MenuTreeParameters;

/**
 * Class TaxonomyMenu.
 *
 * @package Drupal\taxonomy_menu\Controller
 */
class TaxonomyMenu extends ControllerBase {

  /**
   * Render taxonomy links.
   *
   * @return string
   *   Return Hello string.
   */
  public static function renderTaxonomyLinks() {

    $markup = '';

    /*
    // Check current main menu.
    $menu_tree = \Drupal::menuTree();
    $parameters = new MenuTreeParameters();
    $tree = $menu_tree->load('main', $parameters);
    $markup .= var_export($tree, TRUE);
    */

    // Load taxonomy menus.
    $storage = \Drupal::entityManager()->getStorage('taxonomy_menu');
    $taxonomy_menus = $storage->loadMultiple();
    $links = [];

    // Get taxonomy and create menu links from vocabularies.
    foreach ($taxonomy_menus as $taxonomy_menu) {
      $links += $taxonomy_menu->generateTaxonomyLinks([]);
    }

    //$markup .= var_export($links, TRUE);

    return [
        '#type' => 'markup',
        '#markup' => t($markup),
    ];
  }

  /*
   * Generates a menu link id for the taxonomy term.
   */
  public static function generateTaxonomyMenuLinkId($taxonomy_menu, $term) {
    $term_id = $term->id();
    $taxonomy_menu_id = $taxonomy_menu->id();
    return 'taxonomy_menu.menu_link.' . $taxonomy_menu_id . '.' . $term_id;
  }

  /*
   * Generate a render array for taxonomy link.
   */
  public static function generateTaxonomyMenuLink($taxonomy_menu, $term, $base_plugin_definition) {
    $term_id = $term->id();
    $term_url = $term->urlInfo();
    $taxonomy_menu_id = $taxonomy_menu->id();
    $menu_id = $taxonomy_menu->getMenu();

    // Determine parent link.
    // TODO: Evaluate use case of multiple parents (should we make many menu items?)
    $menu_parent_id = NULL;
    $parents = \Drupal::entityManager()->getStorage('taxonomy_term')->loadParents($term_id);

    if (is_array($parents) and count($parents) and $parents[0] != '0') {
      $menu_parent_id = 'taxonomy_menu.menu_link:taxonomy_menu.menu_link.' . $taxonomy_menu_id . '.' . $parents[0]['tid'];
    }

    // TODO: Consider implementing a forced weight based on taxonomy tree.

    // Generate link.
    $arguments = ['taxonomy_term' => $term_id];

    $link = $base_plugin_definition;

    $link += array(
      'id' => self::generateTaxonomyMenuLinkId($taxonomy_menu, $term),
      'title' => $term->label(),
      'description' => $term->getDescription(),
      'menu_name' => $menu_id,
      'metadata' => array(
        'taxonomy_menu_id' => $taxonomy_menu_id,
        'taxonomy_term_id' => $term_id,
      ),
      'route_name' => $term_url->getRouteName(),
      'route_parameters' => $term_url->getRouteParameters(),
      'load arguments'  => $arguments,
      'parent' => $menu_parent_id,
      'provider' => 'taxonomy_menu',
      'class' => 'Drupal\taxonomy_menu\Plugin\Menu\TaxonomyMenuMenuLink',
    );

    return $link;
  }

  /*
   * A reverse lookup of a taxonomy term and it's taxonomy menus.
   */
  public static function getTermTaxonomyMenus($term) {
    $vocab = $term->getVocabularyId();
    return \Drupal::entityManager()->getStorage('taxonomy_menu')->loadByProperties(['vocabulary'=>$vocab]);
  }

  /**
   * Gets array of links based on tax menu.
   */
  public static function getTaxonomyMenuLinks($taxonomy_menu, $base_plugin_definition=[]) {
    $links = [];

    // Load taxonomy terms for tax menu vocab.
    $terms = \Drupal::entityManager()->getStorage('taxonomy_term')->loadTree($taxonomy_menu->getVocabulary());

    $links = [];

    // Create menu links for each term in the vocabulary.
    foreach ($terms as $term_data) {
      $term = \Drupal::entityManager()->getStorage('taxonomy_term')->load($term_data->tid);
      $mlid = \Drupal\taxonomy_menu\Controller\TaxonomyMenu::generateTaxonomyMenuLinkId($taxonomy_menu, $term);
      $links[$mlid] = \Drupal\taxonomy_menu\Controller\TaxonomyMenu::generateTaxonomyMenuLink($taxonomy_menu, $term, $base_plugin_definition);
    }

    return $links;
  }

}
