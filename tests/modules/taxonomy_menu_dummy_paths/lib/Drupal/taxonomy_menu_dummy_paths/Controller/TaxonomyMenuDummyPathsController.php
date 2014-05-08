<?php
/**
 * @file
 * Contains \Drupal\taxonomy_menu_dummy_paths\Controller\TaxonomyMenuDummyPathsController.
 */

namespace Drupal\taxonomy_menu_dummy_paths\Controller;

use Drupal\Core\Controller\ControllerBase;

/**
 * Controller routines for taxonomy_menu_dummy_paths routes.
 */
class TaxonomyMenuDummyPathsController extends ControllerBase {

  /**
   * Menu callback.
   *
   * @param $user_id
   *   The user id of the user whose profile page will be loaded.
   */
  public function TaxonomyMenuDummyPaths($tid = 1, $depth = 2) {
    // We don't want to do anything in particular here.
    return;
  }
}