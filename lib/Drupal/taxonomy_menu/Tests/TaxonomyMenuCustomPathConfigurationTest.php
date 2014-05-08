<?php

/**
 * @file
 * Contains \Drupal\taxonomy_menu\Tests\TaxonomyMenuCustomPathConfigurationTest.
 */

namespace Drupal\taxonomy_menu\Tests;

/**
 * Tests Taxonomy Menu Custom Path configuration options.
 *
 * @TODO Improve the tests by not hardcoding the path to test and generate it
 * instead.
 */
class TaxonomyMenuCustomPathConfigurationTest extends TaxonomyMenuWebTestCase {

  /**
   * Modules to enable.
   *
   * @var array
   */
  public static $modules = array('taxonomy_menu_custom_paths', 'taxonomy_menu_dummy_paths');

  /**
   * Implementation of getInfo().
   */
  public static function getInfo() {
    return array(
      'name' => 'Custom path - Configuration',
      'description' => 'Test custom paths configuration.',
      'group' => 'Taxonomy menu',
    );
  }

  /**
   * Implementation of setUp().
   */
  public function setUp() {
    parent::setUp();

    // Create and login an admin user.
    $admin_user = $this->drupalCreateUser(array('administer taxonomy', 'administer menu', 'bypass node access'));
    $this->drupalLogin($admin_user);

    // Create a vocabulary and a hierarchy of taxonomy terms for it.
    $this->vocabulary = $this->createVocabulary();
    $this->terms_hierarchy = $this->createTermsHierarchy();
  }

  /**
   * Tests if the path is correct without the depth option.
   */
  public function testTaxonomyMenuCustomPathBasePathOption() {
    // Set a base path and submit the vocabulary interface form.
    $edit = array(
      'taxonomy_menu[vocab_parent]' => 'main:0',
      'taxonomy_menu[path]' => 'taxonomy_menu_path_custom',
      'taxonomy_menu[options_custom_path][custom_path_base]' => 'custom_base_path',
    );
    $this->drupalPostForm('admin/structure/taxonomy/manage/' . $this->vocabulary->id(), $edit, t('Save'));
    $this->assertResponse(200);

    // Assert that the base path is in URLs of menu links.
    $mlid = _taxonomy_menu_get_mlid($this->terms_hierarchy[3]->id(), $this->vocabulary->id());
    $menu_link_parent = menu_link_load($mlid);
    $this->assertEqual("custom_base_path/3+4", $menu_link_parent['link_path']);
    $mlid = _taxonomy_menu_get_mlid($this->terms_hierarchy[4]->id(), $this->vocabulary->id());
    $menu_link_leaf = menu_link_load($mlid);
    $this->assertEqual("custom_base_path/4", $menu_link_leaf['link_path']);
  }

  /**
   * Tests if the path is correct with both the base path and the depth option.
   */
  public function testTaxonomyMenuCustomPathDepthOption() {
    // Set a base path and submit the vocabulary interface form.
    $edit = array(
      'taxonomy_menu[vocab_parent]' => 'main:0',
      'taxonomy_menu[path]' => 'taxonomy_menu_path_custom',
      'taxonomy_menu[options_custom_path][custom_path_base]' => 'custom_base_path',
      'taxonomy_menu[options_custom_path][custom_path_depth]' => '2',
    );
    $this->drupalPostForm('admin/structure/taxonomy/manage/' . $this->vocabulary->id(), $edit, t('Save'));
    $this->assertResponse(200);

    // Assert that the depth is used in URLs of menu links.
    $mlid = _taxonomy_menu_get_mlid($this->terms_hierarchy[3]->id(), $this->vocabulary->id());
    $menu_link_parent = menu_link_load($mlid);
    $this->assertEqual("custom_base_path/3+4/2", $menu_link_parent['link_path']);
    $mlid = _taxonomy_menu_get_mlid($this->terms_hierarchy[4]->id(), $this->vocabulary->id());
    $menu_link_leaf = menu_link_load($mlid);
    $this->assertEqual("custom_base_path/4/2", $menu_link_leaf['link_path']);
  }

}