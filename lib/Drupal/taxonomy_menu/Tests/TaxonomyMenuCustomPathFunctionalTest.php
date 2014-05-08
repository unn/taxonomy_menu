<?php

/**
 * @file
 * Contains \Drupal\taxonomy_menu\Tests\TaxonomyMenuCustomPathFunctionalTest.
 */

namespace Drupal\taxonomy_menu\Tests;

/**
 * Tests the taxonomy vocabulary interface.
 */
class TaxonomyMenuCustomPathFunctionalTest extends TaxonomyMenuWebTestCase {

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
      'name' => 'Custom path - Vocabulary interface',
      'description' => 'Test taxonomy menu custom path interface.',
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
   * Saves, edits and deletes a taxonomy vocabulary using the user interface.
   *
   * All the required router paths are already in the database, provided by the
   * helper module taxonomy_menu_dummy_paths.
   */
  public function testTaxonomyMenuCustomPathVocabularyInterface() {
    // Submit without a base path.
    $edit = array(
      'taxonomy_menu[vocab_parent]' => 'main:0',
      'taxonomy_menu[path]' => 'taxonomy_menu_path_custom',
      'taxonomy_menu[options_custom_path][custom_path_base]' => '',
    );
    $this->drupalPostForm('admin/structure/taxonomy/manage/' . $this->vocabulary->id(), $edit, t('Save'));
    $this->assertRaw(t('A base path must be provided when using a custom path.'));

    // Submit with the base path and its respective path being registered.
    $edit['taxonomy_menu[options_custom_path][custom_path_base]'] = 'custom_base_path';
    $this->drupalPostForm('admin/structure/taxonomy/manage/' . $this->vocabulary->id(), $edit, t('Save'));
    $this->assertRaw(t('The Taxonomy menu has been created.'));

    // Submit with base path, depth and respective path not being registered.
    $edit['taxonomy_menu[options_custom_path][custom_path_depth]'] = '5';
    $this->drupalPostForm('admin/structure/taxonomy/manage/' . $this->vocabulary->id(), $edit, t('Save'));
    $this->assertRaw(t('The Taxonomy menu has been updated.'));

    // Submit with base path, depth and respective path not being registered.
    db_delete('router')
      ->condition('path', '/custom_base_path/{tid}/{depth}')
      ->execute();
    $this->drupalPostForm('admin/structure/taxonomy/manage/' . $this->vocabulary->id(), $edit, t('Save'));
    $this->assertRaw(t('The path custom_base_path/%/% is not available in Drupal. This is required to use custom paths.'));

    // Submit with base path, depth and respective path not being registered.
    db_delete('router')
      ->condition('path', '/custom_base_path/{tid}')
      ->execute();
    $this->drupalPostForm('admin/structure/taxonomy/manage/' . $this->vocabulary->id(), $edit, t('Save'));
    $this->assertRaw(t('The path custom_base_path/%/% is not available in Drupal. This is required to use custom paths.'));
  }

}