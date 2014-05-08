<?php

/**
 * @file
 * Definition of Drupal\taxonomy_menu\Tests\TaxonomyMenuWebTestCase.
 */

namespace Drupal\taxonomy_menu\Tests;

use Drupal\taxonomy\Tests\TaxonomyTestBase;
use Drupal\Core\Field\FieldDefinitionInterface;

/**
 * Abstract class for Taxonomy menu testing. All Taxonomy menu tests should
 * extend this class.
 */
abstract class TaxonomyMenuWebTestCase extends TaxonomyTestBase {

  /**
   * Modules to enable.
   *
   * @var array
   */
  public static $modules = array('taxonomy_menu');

  /**
   * A taxonomy vocabulary.
   */
  public $vocabulary;

  /**
   * A hierarchy of taxonomy terms for this vocabulary.
   */
  public $terms_hierarchy;

  /**
   * Asserts that the number of menu links is equal to the number of taxonomy
   * terms for a given menu name.
   *
   * @param $terms
   *   The terms, which are created in this class.
   */
  public function assertEqualNumberTermsMenuLinks($terms_count, $vocabulary, $menu_name) {
    $vid = $vocabulary->id();
    // Building a query getting the number of menu links for this vocabulary.
    $query = db_select('menu_links', 'ml')
      ->fields('ml')
      ->condition('ml.module', 'taxonomy_menu')
      ->condition('ml.menu_name', $menu_name);
    $query->join('taxonomy_menu', 'tm', 'ml.mlid = tm.mlid');
    $query->condition('tm.vid', $vid, '=');
    $query_count = $query->countQuery()->execute()->fetchField();
    $message = $query_count . ' menu links were found for the ' . $terms_count . ' taxonomy terms of vocabulary ' . $vocabulary->name . '.';
    $this->assertEqual($terms_count, $query_count, $message);
  }

  /**
   * Creates a hierarchy of taxonomy terms for the vocabulary defined in the
   * current class.
   *
   * @return
   *   An array of 7 hierarchised taxonomy term objects.
   *
   *   The hierarchy is as follow:
   *     terms[1]         | depth: 0
   *     -- terms[2]      | depth: 1
   *     -- terms[3]      | depth: 1
   *     ---- terms[4]    | depth: 2
   *     -- terms[5]      | depth: 1
   *     terms[6]         | depth: 0
   *     -- terms[7]      | depth: 1
   *
   * @TODO Add multiple parents when taxonomy_menu can deal with it.
   */
  public function createTermsHierarchy() {
    $terms = array();
    for ($i = 1; $i < 8; $i++) {
      $terms[$i] = $this->createTerm($this->vocabulary);
    }

    // Set the hierarchy by adding parent terms.
    $terms[2]->parent = array($terms[1]->id());
    $terms[2]->save();
    $terms[3]->parent = array($terms[1]->id());
    $terms[3]->save();
    $terms[4]->parent = array($terms[3]->id());
    $terms[4]->save();
    $terms[5]->parent = array($terms[1]->id());
    $terms[5]->save();
    $terms[7]->parent = array($terms[6]->id());
    $terms[7]->save();

    return $terms;
  }

  /**
   * Fetches the menu item from the database and compare it to the specified
   * array.
   *
   * @param $mlid
   *   The identifier of a menu link.
   * @param $expected_item
   *   An array containing properties to verify.
   */
  public function assertMenuLink($mlid, array $expected_item) {
    // Retrieve menu link.
    $item = db_query('SELECT * FROM {menu_links} WHERE mlid = :mlid', array(':mlid' => $mlid))->fetchAssoc();
    $options = unserialize($item['options']);
    if (!empty($options['query'])) {
      $item['link_path'] .= '?' . drupal_http_build_query($options['query']);
    }
    if (!empty($options['fragment'])) {
      $item['link_path'] .= '#' . $options['fragment'];
    }
    foreach ($expected_item as $key => $value) {
      $this->assertEqual($item[$key], $value, t('Parameter %key had expected value.', array('%key' => $key)));
    }
  }

  /**
   * Adds a taxonomy reference field to a content type and creates a number of
   * nodes that references different taxonomy terms
   *
   * @param $type string
   *   The content type's machine name to add a term reference field to.
   * @param $terms_index array
   *   An array of term indexes from the terms hierarchy of this class. Each
   *   index will be used to attach a node to this term. Indexes can be duplicated
   *   in order to attach several nodes to the same term.
   */
  public function setUpTermReferenceAndNodes($type, $terms_index) {
    $this->field_name = 'taxonomy_' . $this->vocabulary->id();

    entity_create('field_config', array(
      'name' => $this->field_name,
      'entity_type' => 'node',
      'type' => 'taxonomy_term_reference',
      'cardinality' => FieldDefinitionInterface::CARDINALITY_UNLIMITED,
      'settings' => array(
        'allowed_values' => array(
          array('vocabulary' => $this->vocabulary->id(), 'parent' => 0),
        ),
      ),
    ))->save();
    entity_create('field_instance_config', array(
      'field_name' => $this->field_name,
      'bundle' => $type,
      'entity_type' => 'node',
    ))->save();

    entity_get_form_display('node', 'article', 'default')
      ->setComponent($this->field_name, array(
        'type' => 'options_select',
      ))
      ->save();
    entity_get_display('node', 'article', 'default')
      ->setComponent($this->field_name, array(
        'type' => 'taxonomy_term_reference_link',
      ))
      ->save();

    // Create nodes that reference each term represented by their indexes.
    foreach ($terms_index as $index) {
      $edit = array();
      $edit['title[0][value]'] = $this->randomName();
      $edit['body[0][value]'] = $this->randomName();
      $edit[$this->field_name . '[]'] = $this->terms_hierarchy[$index]->id();
      $this->drupalPostForm('node/add/article', $edit, t('Save'));
    }
  }
}