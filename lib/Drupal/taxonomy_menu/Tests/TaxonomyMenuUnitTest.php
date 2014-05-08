<?php

/**
 * @file
 * Contains \Drupal\taxonomy_menu\Tests\TaxonomyMenuUnitTest.
 */

namespace Drupal\taxonomy_menu\Tests;

/**
 * Tests for taxonomy vocabulary functions.
 */
class TaxonomyMenuUnitTest extends TaxonomyMenuWebTestCase {

  /**
   * Modules to enable.
   *
   * @var array
   */
  public static $modules = array('taxonomy_menu');

  /**
   * Implementation of getInfo().
   */
  public static function getInfo() {
    return array(
      'name' => 'CRUD functions',
      'description' => 'Test CRUD functions',
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
   * Tests CRUD functions.
   */
  public function testTaxonomyMenuCRUD() {
    $menu_name = 'main';
    $vocabulary = $this->vocabulary;
    $terms = $this->terms_hierarchy;
    $hierarchy_term = $this->terms_hierarchy[3];
    $vid = $this->vocabulary->id();

    // Ensure that the taxonomy vocabulary form is successfully submitted.
    $edit['taxonomy_menu[vocab_parent]'] = $menu_name . ':0';
    $this->drupalPostForm('admin/structure/taxonomy/manage/' . $vid, $edit, t('Save'));
    $this->assertResponse(200);

    // Ensure that the same number of menu links are created from the taxonomy
    // terms of the vocabulary.
    $this->assertEqualNumberTermsMenuLinks(count($terms), $vocabulary, $menu_name);

    // Ensure that the menu link is updated when the taxonomy term is updated.
    $new_name = $this->randomName();
    $hierarchy_term->name = $new_name;
    $hierarchy_term->save();
    $this->drupalGet('admin/structure/menu/manage/' . $menu_name);
    $this->assertLink($new_name);

    // Ensure that the menu link is deleted when the taxonomy term is deleted.
    // Hierarchy term [3] has 1 children, if we delete it, 2 taxonomy terms
    // should be deleted.
    $orig_mlid = _taxonomy_menu_get_mlid($hierarchy_term->id(), $vid);
    entity_load('taxonomy_term', $hierarchy_term->id())->delete();
    $this->assertEqualNumberTermsMenuLinks(count($terms) - 2, $vocabulary, $menu_name);

    $menu_link = menu_link_load($orig_mlid);
    $message = 'The menu link ' . $orig_mlid . ' associated to the term ' . $hierarchy_term->id() . ' could not be found.';
    $this->assertFalse($menu_link, $message);

    $mlid = _taxonomy_menu_get_mlid($hierarchy_term->id(), $vid);
    $message = 'The ( mlid = ' . $orig_mlid . ' / tid = ' . $hierarchy_term->id() . ') association could not be found in {taxonomy_menu} table.';
    $this->assertFalse($mlid, $message);

    // Ensure that all menu links and all associations in {taxonomy_menu} table
    // are deleted when a vocabulary is deleted.
    $mlids = _taxonomy_menu_get_menu_items($vid);
    $vocabulary->delete();
    $this->assertEqualNumberTermsMenuLinks(0, $vocabulary, $menu_name);
  }

  /**
   * Tests the hierarchy of menu links in a menu.
   */
  public function testTaxonomyMenuTermsHierarchy() {
    $vid = $this->vocabulary->id();
    $edit = array();

    // Settings
    $edit['taxonomy_menu[vocab_parent]'] = 'main:0';
    $this->drupalPostForm('admin/structure/taxonomy/manage/' . $vid, $edit, t('Save'));
    $this->assertResponse(200);

    // Given a taxonomy term, which id is tid:
    //   - ptid      -->  - plid
    //     - tid     -->    - mlid
    // Do the following verification for each term in the hierarchy; the ending
    // plid determined by the methods below should be equal.
    //   - method1: tid --> mlid --> plid
    //   - method2: tid --> ptid --> plid
    foreach ($this->terms_hierarchy as $term) {
      // 1. Get plid by getting the associated mlid for the term tid.
      $mlid = _taxonomy_menu_get_mlid($term->id(), $vid);
      if ($mlid) {
        $menu_link = menu_link_load($mlid);
        $plid_from_mlid = $menu_link['plid'];
        // 2. Get plid by getting the associated mlid for the parent term ptid.
        // We don't handle multiple parents, break after first one.
        $parents = taxonomy_term_load_parents($term->id());
        if (!empty($parents)) {
          foreach ($parents as $ptid => $parent) {
            $plid_from_ptid = _taxonomy_menu_get_mlid($ptid, $vid);
            // Assert that both plid found by the two different methods are equal.
            $message = 'Parent mlids from taxonomy term ' . $term->id() . ' are a match.';
            $this->assertEqual($plid_from_mlid, $plid_from_ptid, $message);
            break;
          }
        }
        else {
          // Current term has no parent term. This means that the name of the
          // vocabulary should be associated to the 'navigation' root.
          // Menu link of the current term as defined by taxonomy menu table.
          $this->assertEqual($menu_link['plid'], 0);
        }
      }
      else {
        $this->fail("mlid for taxonomy term " . $term->id() . " could not be found.");
      }
    }
  }

  /**
   * Tests creation of menu links in a custom menu.
   */
  public function testTaxonomyMenuCustomMenu() {
    $vocabulary = $this->vocabulary;
    $terms = $this->terms_hierarchy;

    $custom_menu_name = $this->randomName(16);
    $menu_machine_name = substr(hash('sha256', $custom_menu_name), 0, MENU_MAX_MENU_NAME_LENGTH_UI);

    // Submit the menu creation form.
    $menu_edit = array(
      'id' => $menu_machine_name,
      'description' => '',
      'label' =>  $custom_menu_name,
    );
    $this->drupalPostForm('admin/structure/menu/add', $menu_edit, t('Save'));
    $this->assertResponse(200);

    // Submit the vocabulary edit form.
    $vocab_edit = array();
    $vocab_edit['taxonomy_menu[vocab_parent]'] = $menu_machine_name . ':0';
    $this->drupalPostForm('admin/structure/taxonomy/manage/' . $vocabulary->id(), $vocab_edit, 'Save');
    $this->assertResponse(200);

    // Check that the menu links were created in the custom menu.
    $this->assertEqualNumberTermsMenuLinks(count($terms), $vocabulary, $menu_machine_name);
  }

}