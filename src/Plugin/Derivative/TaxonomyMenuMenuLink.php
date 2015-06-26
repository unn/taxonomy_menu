<?php

/**
 * @file
 * Contains \Drupal\taxonomy_menu\Plugin\Derivative\TaxonomyMenuMenuLink.
 */

namespace Drupal\taxonomy_menu\Plugin\Derivative;

use Drupal\Component\Plugin\Derivative\DeriverBase;
use Drupal\Core\Plugin\Discovery\ContainerDeriverInterface;
use Drupal\Core\Entity\EntityStorageInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Provides menu links for Taxonomy Menus.
 *
 * @see \Drupal\taxonomy_menu\Plugin\Menu\TaxonomyMenuMenuLink
 */
class ViewsMenuLink extends DeriverBase implements ContainerDeriverInterface {

  /**
   * The taxonomy menu storage.
   *
   * @var \Drupal\Core\Entity\EntityStorageInterface
   */
  protected $taxonomyMenuStorage;

  /**
   * Constructs a \Drupal\views\Plugin\Derivative\ViewsLocalTask instance.
   *
   * @param \Drupal\Core\Entity\EntityStorageInterface $view_storage
   *   The view storage.
   */
  public function __construct(EntityStorageInterface $taxonomy_menu_storage) {
    $this->taxonomyMenuStorage = $taxonomy_menu_storage;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container, $base_plugin_id) {
    return new static(
      $container->get('entity.manager')->getStorage('taxonomy_menu')
    );
  }

  /**
   * {@inheritdoc}
   */
  public function getDerivativeDefinitions($base_plugin_definition) {
    $links = array();
    $views = Views::getApplicableViews('uses_menu_links');

    foreach ($views as $data) {
      list($view_id, $display_id) = $data;
      /** @var \Drupal\views\ViewExecutable $executable */
      $executable = $this->viewStorage->load($view_id)->getExecutable();

      if ($result = $executable->getMenuLinks($display_id)) {
        foreach ($result as $link_id => $link) {
          $links[$link_id] = $link + $base_plugin_definition;
        }
      }
    }

    return $links;
  }

}
