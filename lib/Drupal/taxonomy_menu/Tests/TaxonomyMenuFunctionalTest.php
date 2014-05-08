<?php

/**
 * @file
 * Contains \Drupal\taxonomy_menu\Tests\TaxonomyMenuFunctionalTest.
 */

namespace Drupal\taxonomy_menu\Tests;

/**
 * Tests the taxonomy vocabulary interface.
 */
class TaxonomyMenuFunctionalTest extends TaxonomyMenuWebTestCase {

  /**
   * Implementation of getInfo().
   */
  public static function getInfo() {
    return array(
      'name' => 'Vocabulary interface',
      'description' => 'Test the taxonomy menu vocabulary interface.',
      'group' => 'Taxonomy menu',
    );
  }

  /**
   * Implementation of setUp().
   */
  function setUp() {
    parent::setUp();

    // Create and login an admin user.
    $admin_user = $this->drupalCreateUser(array('administer taxonomy', 'administer menu', 'bypass node access'));
    $this->drupalLogin($admin_user);

    // Create a vocabulary and a hierarchy of taxonomy terms for it.
    $this->vocabulary = $this->createVocabulary();
    $this->terms_hierarchy = $this->createTermsHierarchy();
  }

  /**
   * Save, edit and delete a taxonomy vocabulary using the user interface.
   */
  public function testTaxonomyMenuVocabularyInterface() {
    $menu_name = 'account';
    $vid = $this->vocabulary->id();

    // Visit the main taxonomy administration page.
    $this->drupalGet('admin/structure/taxonomy/manage/' . $vid);
    // Options for the taxonomy vocabulary edit form.
    $edit = array();
    // Try to submit a vocabulary when menu location is a root menu item.
    $edit['taxonomy_menu[vocab_parent]'] = $menu_name . ':0';
    $this->drupalPostForm(NULL, $edit, t('Save'));
    $this->assertRaw(t('The Taxonomy menu has been created.'));
    // Try to re-submit a vocabulary when an option has changed.
    $edit['taxonomy_menu[options_structure][hide_empty_terms]'] = TRUE;
    $this->drupalPostForm('admin/structure/taxonomy/manage/' . $vid, $edit, t('Save'));
    $this->assertRaw(t('The Taxonomy menu has been updated.'));
    // Try to re-submit a vocabulary without changing any option.
    $this->drupalPostForm('admin/structure/taxonomy/manage/' . $vid, $edit, t('Save'));
    $this->assertRaw(t('The Taxonomy menu was not updated. Nothing to update.'));
    // Try to submit a vocabulary removing the menu location.
    $edit['taxonomy_menu[vocab_parent]'] = '0';
    $this->drupalPostForm('admin/structure/taxonomy/manage/' . $vid, $edit, t('Save'));
    $this->assertRaw(t('The Taxonomy menu has been removed.'));
  }

  /**
   * Re-order terms on the terms' overview page.
   */
  public function testTaxonomyMenuTermsOverviewInterface() {
    $this->terms_hierarchy = $this->createTermsHierarchy($this->vocabulary->id());
    // Last term of our hierarchy.
    $term7 = $this->terms_hierarchy[7];
    // Submit the main taxonomy administration page.
    $edit = array(
      'taxonomy_menu[vocab_parent]' => 'main:0',
      'taxonomy_menu[sync]' => TRUE,
    );
    $this->drupalPostForm('admin/structure/taxonomy/manage/' . $this->vocabulary->id(), $edit, t('Save'));
    // Visit the terms overview page.
    $this->drupalGet('admin/structure/taxonomy/manage/' . $this->vocabulary->id() . '/overview');
    // Take last term, place it on top and save.
    $edit = array(
      'terms[tid:' . $term7->id() . ':0][term][tid]' => $term7->id(),
      'terms[tid:' . $term7->id() . ':0][term][parent]' => 0,
      'terms[tid:' . $term7->id() . ':0][term][depth]' => 0,
      'terms[tid:' . $term7->id() . ':0][weight]' => -5,
    );
    $this->drupalPostForm(NULL, $edit, t('Save'));
    $this->assertRaw(t('The Taxonomy menu has been updated.'));
    // Test "Reset to alphabetical".
    $this->drupalPostForm('admin/structure/taxonomy/manage/' . $this->vocabulary->id() . '/overview', array(), t('Reset to alphabetical'));
    $this->drupalPostForm(NULL, array(), t('Reset to alphabetical'));
    $this->assertRaw(t('The Taxonomy menu has been updated.'));
  }

  /**
   * Saves, edits and deletes a term using the user interface.
   */
  public function testTaxonomyMenuTermInterface() {
    $menu_name = 'main';

    // Create a taxonomy menu.
    $vocab_settings['taxonomy_menu[vocab_parent]'] = $menu_name . ':0';
    $this->drupalPostForm('admin/structure/taxonomy/manage/' . $this->vocabulary->id(), $vocab_settings, t('Save'));

    // Create a new term from the interface.
    $term_settings = array(
      'name[0][value]' => $this->randomName(12),
      'description[0][value]' => $this->randomName(100),
    );
    $this->drupalPostForm('admin/structure/taxonomy/manage/' . $this->vocabulary->id() . '/add', $term_settings, t('Save'));
    $terms = taxonomy_term_load_multiple_by_name($term_settings['name[0][value]']);
    $term = reset($terms);
    $this->assertRaw(t('Added term %term to taxonomy menu %menu_name.', array('%term' => $term_settings['name[0][value]'], '%menu_name' => $menu_name)));

    // Update an existing term from the interface.
    $new_term_settings = array(
      'name[0][value]' => $this->randomName(12),
      'description[0][value]' => $this->randomName(100),
    );
    $this->drupalPostForm('taxonomy/term/' . $term->id() . '/edit', $new_term_settings, t('Save'));
    $this->assertRaw(t('Updated term %term in taxonomy menu %menu_name.', array('%term' => $new_term_settings['name[0][value]'], '%menu_name' => $menu_name)));

    // Delete an existing term from the interface.
    $this->clickLink(t('Delete'));
    $this->drupalPostForm(NULL, NULL, t('Delete'));
    $this->assertRaw(t('Deleted term %term from taxonomy menu %menu_name.', array('%term' => $new_term_settings['name[0][value]'], '%menu_name' => $menu_name)));
  }

}