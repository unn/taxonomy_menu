<?php

/**
 * @file
 * Contains \Drupal\taxonomy_menu\Tests\TaxonomyMenuConfigurationTest.
 */

namespace Drupal\taxonomy_menu\Tests;

/**
 * Tests Taxonomy menu configuration options.
 */
class TaxonomyMenuConfigurationTest extends TaxonomyMenuWebTestCase {

  /**
   * Implementation of getInfo().
   */
  public static function getInfo() {
    return array(
      'name' => 'Configuration options',
      'description' => 'Test configuration options.',
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
   * Tests Taxonommy Menu sync option.
   */
  public function testTaxonomyMenuSyncOption() {
    $vid = $this->vocabulary->id();

    // Set settings (no sync on main).
    $edit = array(
      'taxonomy_menu[vocab_parent]' => 'main:0',
      'taxonomy_menu[sync]' => FALSE,
    );
    $this->drupalPostForm('admin/structure/taxonomy/manage/' . $vid, $edit, t('Save'));
    $this->assertResponse(200);

    // Prepare changes.
    $new_name = $this->randomName(12);
    // Arbitrary term from hierarchy.
    $test_term = $this->terms_hierarchy[3];
    $test_term->setName($new_name);
    $test_term->save();

    $mlid = _taxonomy_menu_get_mlid($test_term->id(), $this->vocabulary->id());
    $menu_link = menu_link_load($mlid);
    $this->assertNotEqual($new_name, $menu_link['link_title']);

    // Switch to sync option on and save.
    $edit['taxonomy_menu[sync]'] = TRUE;
    $this->drupalPostForm('admin/structure/taxonomy/manage/' . $vid, $edit, t('Save'));
    $this->assertResponse(200);
    $mlid = _taxonomy_menu_get_mlid($test_term->id(), $this->vocabulary->id());
    $menu_link = menu_link_load($mlid);
    $this->assertEqual($new_name, $menu_link['link_title']);
  }

  /**
   * Tests Taxonommy Menu expand option.
   */
  public function testTaxonomyMenuExpandedOption() {
    $vid = $this->vocabulary->id();

    // Set settings on expanded.
    $edit = array(
      'taxonomy_menu[vocab_parent]' => 'main:0',
      'taxonomy_menu[options_structure][expanded]' => TRUE,
    );
    $this->drupalPostForm('admin/structure/taxonomy/manage/' . $vid, $edit, t('Save'));

    // Build the base query.
    $base_query = db_select('menu_links', 'ml');
    $base_query->join('taxonomy_menu', 'tm', 'ml.mlid = tm.mlid');
    $base_query->fields('ml')
      ->condition('tm.vid', $this->vocabulary->id())
      ->condition('ml.module', 'taxonomy_menu');

    // Assert that menu links are expanded.
    $query = $base_query->condition('ml.expanded', TRUE);
    $row_count = $query->countQuery()->execute()->fetchField();
    $this->assertEqual(count($this->terms_hierarchy), $row_count);

    // Set settings on not expanded.
    $edit = array(
      'taxonomy_menu[vocab_parent]' => 'main:0',
      'taxonomy_menu[options_structure][expanded]' => FALSE,
    );
    $this->drupalPostForm('admin/structure/taxonomy/manage/' . $vid, $edit, t('Save'));

    // Assert that menu links are not expanded anymore.
    $query = $base_query->condition('ml.expanded', FALSE);
    $row_count = $query->countQuery()->execute()->fetchField();
    $this->assertEqual(0, $row_count);
  }

  /**
   * Tests Taxonommy Menu "Term description" option.
   */
  public function testTaxonomyMenuTermDescriptionOption() {
    $vid = $this->vocabulary->id();

    // Set settings on expanded.
    $edit = array(
      'taxonomy_menu[vocab_parent]' => 'main:0',
      'taxonomy_menu[options_markup][term_item_description]' => FALSE,
      'taxonomy_menu[options_markup][display_title_attr]' => TRUE,
    );
    $this->drupalPostForm('admin/structure/taxonomy/manage/' . $vid, $edit, t('Save'));

    // Assert that menu links does not have the term description.
    $term = $this->terms_hierarchy[3];
    $mlid = _taxonomy_menu_get_mlid($term->id(), $this->vocabulary->id());
    $menu_link = menu_link_load($mlid);
    $menu_link_title = $menu_link['options']['attributes']['title'];
    $this->assertEqual($menu_link_title, '');

    // Assert that menu links does have the term description, when the option is
    // checked.
    $edit = array(
      'taxonomy_menu[vocab_parent]' => 'main:0',
      'taxonomy_menu[options_markup][term_item_description]' => TRUE,
      'taxonomy_menu[options_markup][display_title_attr]' => TRUE,
    );
    $this->drupalPostForm('admin/structure/taxonomy/manage/' . $vid, $edit, t('Save'));
    $menu_link = menu_link_load($mlid);
    $menu_link_title = $menu_link['options']['attributes']['title'];
    $this->assertEqual($menu_link_title, trim($term->getDescription()));

    // Assert that menu links does not have the term description, when the option
    // for displaying a description is on but the display title option is off.
    $edit = array(
      'taxonomy_menu[vocab_parent]' => 'main:0',
      'taxonomy_menu[options_markup][term_item_description]' => TRUE,
      'taxonomy_menu[options_markup][display_title_attr]' => FALSE,
    );
    $this->drupalPostForm('admin/structure/taxonomy/manage/' . $vid, $edit, t('Save'));
    $menu_link = menu_link_load($mlid);
    $menu_link_title = $menu_link['options']['attributes']['title'];
    $this->assertEqual($menu_link_title, '');
  }

  /**
   * Tests Taxonommy Menu "Flatten" option.
   */
  public function testTaxonomyMenuFlattenOption() {
    $vid = $this->vocabulary->id();

    // Set settings.
    $edit = array(
      'taxonomy_menu[vocab_parent]' => 'main:0',
      'taxonomy_menu[options_structure][flat]' => TRUE,
    );
    $this->drupalPostForm('admin/structure/taxonomy/manage/' . $vid, $edit, t('Save'));

    // Assert that all of the menu links have no children with the root being
    // the menu.
    $query = db_select('menu_links', 'ml');
    $query->join('taxonomy_menu', 'tm', 'ml.mlid = tm.mlid');
    $query->fields('ml');
    $query
      ->condition('tm.vid', $this->vocabulary->id())
      ->condition('ml.menu_name', 'main')
      ->condition('ml.module', 'taxonomy_menu')
      ->condition('ml.has_children', 0)
      ->condition('ml.plid', 0);
    $row_count = $query->countQuery()->execute()->fetchField();
    $this->assertEqual(count($this->terms_hierarchy), $row_count);

    // Assert that all of the menu links have no children with the root being
    // a menu item.
    $menu_link = entity_create('menu_link', array(
      'menu_name'  => 'main',
      'weight'     => 0,
      'link_title' => 'test',
      'link_path'  => '<front>',
      'module' => 'taxonomy_menu',
    ));
    menu_link_save($menu_link);
    $mlid = entity_load_by_uuid('menu_link', $menu_link->uuid)->mlid;
    menu_cache_clear_all('main');

    $edit = array(
      'taxonomy_menu[vocab_parent]' => 'main:' . $mlid,
      'taxonomy_menu[options_structure][flat]' => TRUE,
    );
    $this->drupalPostForm('admin/structure/taxonomy/manage/' . $vid, $edit, t('Save'));

    $query = db_select('menu_links', 'ml');
    $query->join('taxonomy_menu', 'tm', 'ml.mlid = tm.mlid');
    $query->fields('ml');
    $query
      ->condition('tm.vid', $this->vocabulary->id())
      ->condition('ml.menu_name', 'main')
      ->condition('ml.module', 'taxonomy_menu')
      ->condition('ml.has_children', 0)
      ->condition('ml.plid', $mlid);
    $row_count = $query->countQuery()->execute()->fetchField();
    $this->assertEqual(count($this->terms_hierarchy), $row_count);
  }

  /**
   * Tests Taxonommy Menu "Hide Empty terms" option.
   */
  public function testTaxonomyMenuHideEmptyTerms() {
    $vid = $this->vocabulary->id();

    // Create several nodes and attach them to different terms of our hierarchy
    // in order to match the following scheme.
    /** terms[1]         | depth: 0 | 0 node  -> hidden
     * -- terms[2]       | depth: 1 | 0 node  -> hidden
     * -- terms[3]       | depth: 1 | 2 nodes -> displayed
     * ---- terms[4]     | depth: 2 | 0 node  -> hidden
     * -- terms[5]       | depth: 1 | 1 node  -> displayed
     * terms[6]          | depth: 0 | 0 node  -> hidden
     * -- terms[7]       | depth: 1 | 0 node  -> hidden   */

    $this->setUpTermReferenceAndNodes('article', array(3, 3, 5));

    // Set settings (don't hide empty terms) and save.
    $edit = array(
      'taxonomy_menu[vocab_parent]' => 'main:0',
      'taxonomy_menu[options_structure][hide_empty_terms]' => FALSE,
    );
    $this->drupalPostForm('admin/structure/taxonomy/manage/' . $vid, $edit, t('Save'));

    // Assert that all links are displayed.
    foreach ($this->terms_hierarchy as $term) {
      $mlid = _taxonomy_menu_get_mlid($term->id(), $vid);
      if ($mlid) {
        $this->assertMenuLink($mlid, array('hidden' => FALSE));
      }
      else {
        $this->fail('No mlid could be found for the term ' . $term->id());
      }
    }

    // Set settings (hide empty terms) and save.
    $edit['taxonomy_menu[options_structure][hide_empty_terms]'] = TRUE;
    $this->drupalPostForm('admin/structure/taxonomy/manage/' . $vid, $edit, t('Save'));

    // Assert that the hidden property of the taxonomy menu's menu links are
    // set according to the scheme.
    $visible_terms_index = array(3, 5);
    $index = 1;
    foreach ($this->terms_hierarchy as $term) {
      $mlid = _taxonomy_menu_get_mlid($term->id(), $vid);
      if ($mlid) {
        if (in_array($index, $visible_terms_index)) {
          $this->assertMenuLink($mlid, array('hidden' => FALSE));
        }
        else {
          $this->assertMenuLink($mlid, array('hidden' => TRUE));
        }
      }
      else {
        $this->fail('No mlid could be found for the term ' . $term->tid);
      }
      $index++;
    }
  }

  /**
   * Tests Taxonommy Menu "Node count" option.
   *
   * @TODO Add a test for recursive count.
   */
  public function testTaxonomyMenuCountNodes() {
    $vid = $this->vocabulary->id();

    /*
      Create several nodes and attach them to different terms of our hierarchy
      in order to match the following scheme. We don't use "hide empty terms".
      option.

      terms[1]          | depth: 0 | 0 node attached
      -- terms[2]       | depth: 1 | 0 node attached
      -- terms[3]       | depth: 1 | 2 nodes attached
      ---- terms[4]     | depth: 2 | 2 nodes attached
      -- terms[5]       | depth: 1 | 1 node attached
      terms[6]          | depth: 0 | 1 node attached
      -- terms[7]       | depth: 1 | 0 node attached

      We expect the following result for number of items:
      - terms[1]: count is 0
      - terms[3]: count is 2
      - terms[4]: count is 2
      - terms[5]: count is 1
      - terms[6]: count is 1
      - Others  : count is 0
    */

    $this->setUpTermReferenceAndNodes('article', array(3, 3, 4, 4, 5, 6));

    // Set settings and save.
    $edit = array(
      'taxonomy_menu[vocab_parent]' => 'main:0',
      'taxonomy_menu[options_structure][hide_empty_terms]' => FALSE,
      'taxonomy_menu[options_markup][display_num]' => TRUE,
    );
    $this->drupalPostForm('admin/structure/taxonomy/manage/' . $vid, $edit, t('Save'));

    // Assert that the count is correct in the menu links according to the scheme.
    $index = 1;
    $positive_count_terms_index = array(3, 4, 5, 6);
    $visible_item = array('hidden' => FALSE);
    foreach ($this->terms_hierarchy as $term) {
      $mlid = _taxonomy_menu_get_mlid($term->id(), $vid);
      $menu_link = menu_link_load($mlid);
      if ($mlid) {
        switch ($index) {
          case '3':
          case '4':
            $count = 2;
            break;
          case '5':

          case '6':
            $count = 1;
            break;

          default:
            $count = 0;
            break;
        }
        if (in_array($index, $positive_count_terms_index)) {
          $this->assertMenuLink($mlid, array('link_title' => $term->getName() . ' (' . $count . ')'));
        }
        else {
          $this->assertMenuLink($mlid, array('link_title' => $term->getName()));
        }
      }
      else {
        $this->fail('No mlid could be found for the term ' . $term->id());
      }
      $index++;
    }
  }
}
