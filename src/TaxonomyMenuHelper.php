<?php

/**
 * @file
 * Contains Drupal\taxonomy_menu\Controller\TaxonomyMenu.
 */

namespace Drupal\taxonomy_menu;

use Drupal\Core\Entity\EntityManagerInterface;
use Drupal\Core\Menu\MenuLinkManagerInterface;
use Drupal\taxonomy\TermInterface;

/**
 * Class TaxonomyMenu.
 *
 * @package Drupal\taxonomy_menu
 */
class TaxonomyMenuHelper {

  /**
   * @var \Drupal\Core\Entity\EntityStorageInterface
   */
  protected $menuStorage;

  /**
   * @var \Drupal\Core\Menu\MenuLinkManagerInterface
   */
  protected $manager;

  public function __construct(EntityManagerInterface $entity_manager, MenuLinkManagerInterface $manager) {
    $this->menuStorage = $entity_manager->getStorage('taxonomy_menu');
    $this->manager = $manager;
  }

  /**
   * Render taxonomy links.
   *
   * @return string
   *   Return Hello string.
   */
  public function renderTaxonomyLinks() {

    $markup = '';

    /*
    // Check current main menu.
    $menu_tree = \Drupal::menuTree();
    $parameters = new MenuTreeParameters();
    $tree = $menu_tree->load('main', $parameters);
    $markup .= var_export($tree, TRUE);
    */

    // This is not a thing...
    /*
    $taxonomy_menus = $this->menuStorage->loadMultiple();
    $links = [];

    // Get taxonomy and create menu links from vocabularies.
    foreach ($taxonomy_menus as $taxonomy_menu) {
      $links += $taxonomy_menu->generateTaxonomyLinks([]);
    }

    //$markup .= var_export($links, TRUE);

     */
    return [
        '#type' => 'markup',
        '#markup' => t($markup),
    ];
  }

  /**
   * A reverse lookup of a taxonomy term menus by vocabulary.
   *
   * @return \Drupal\taxonomy_menu\TaxonomyMenuInterface[]
   */
  public function getTermMenusByVocabulary($vid) {
    return $this->menuStorage->loadByProperties(['vocabulary'=>$vid]);
  }

  /**
   * Create menu entries associate with the vocabulary of this term.
   *
   * @param \Drupal\taxonomy\TermInterface $term
   */
  public function generateTaxonomyMenuEntries(TermInterface $term, $rebuild_all = TRUE) {
    // Load relevant taxonomy menus.
    $tax_menus = $this->getTermMenusByVocabulary($term->getVocabularyId());
    foreach ($tax_menus as $menu) {
      foreach ($menu->getLinks([], TRUE) as $plugin_id => $plugin_def) {
        if (!$rebuild_all) {
          $plugin_id_parts = explode('.', $plugin_id);
          $term_id = array_pop($plugin_id_parts);
          if ($term->id() != $term_id) {
            continue;
          }
        }
        if ($this->manager->hasDefinition($plugin_id)) {
          $this->manager->updateDefinition($plugin_id, $plugin_def);
        }
        else {
          $this->manager->addDefinition($plugin_id, $plugin_def);
        }
      }
    }
  }

  /**
   * Update menu entries associate with the vocabulary of this term.
   *
   * @param \Drupal\taxonomy\TermInterface $term
   */
  public function updateTaxonomyMenuEntries(TermInterface $term, $rebuild_all = TRUE) {
    // Load relevant taxonomy menus.
    $tax_menus = $this->getTermMenusByVocabulary($term->getVocabularyId());
    foreach ($tax_menus as $menu) {
      foreach ($menu->getLinks([], TRUE) as $plugin_id => $plugin_def) {
        drupal_set_message(print_r($menu->getVocabulary(), TRUE));
        if (!$rebuild_all) {
          $plugin_id_explode = explode('.', $plugin_id);
          $term_id = array_pop($plugin_id_explode);
          if ($term->id() != $term_id) {
            continue;
          }
        }
        $this->manager->updateDefinition($plugin_id, $plugin_def, FALSE);
      }
    }
  }

  /**
   * Remove menu entries associate with the vocabulary of this term.
   *
   * @param \Drupal\taxonomy\TermInterface $term
   */
  public function removeTaxonomyMenuEntries(TermInterface $term, $rebuild_all = TRUE) {
    // Load relevant taxonomy menus.
    $tax_menus = $this->getTermMenusByVocabulary($term->getVocabularyId());
    foreach ($tax_menus as $menu) {
      foreach (array_keys($menu->getLinks([], TRUE)) as $plugin_id) {
        if (!$rebuild_all) {
          $plugin_id_parts = explode('.', $plugin_id);
          $term_id = array_pop($plugin_id_parts);
          if ($term->id() != $term_id) {
            continue;
          }
        }
        $this->manager->removeDefinition($plugin_id, FALSE);
      }
    }
  }

}
