<?php

namespace Drupal\taxonomy_menu\Tests\Functional;

use Drupal\Tests\BrowserTestBase;

/**
 * Tests that the Taxonomy menu links are translatable.
 *
 * @group taxonomy_menu
 */
class TaxonomyMenuLinkLanguageTest extends BrowserTestBase {
  /**
   * Modules to enable.
   *
   * @var array
   */
  protected static $modules = [
    'taxonomy',
    'taxonomy_menu',
    'language',
    'menu_ui',
    'content_translation',
    'config_translation',
    'locale',
  ];

  /**
   * {@inheritdoc}
   */
  protected $defaultTheme = 'stark';

  /**
   * Vocabulary for testing.
   *
   * @var \Drupal\taxonomy\VocabularyInterface
   */
  protected $vocabulary;

  /**
   * {@inheritdoc}
   */
  public function setUp() {
    parent::setUp();

    $perms = [
      'access content',
      'administer site configuration',
      'administer taxonomy',
      'administer languages',
      'administer menu',
      'administer content translation',
      'create content translations',
      'update content translations',
      'translate any entity',
    ];
    // Create an administrative user.
    $this->drupalLogin($this->drupalCreateUser($perms));

    // Enable German language.
    $this->drupalGet('admin/config/regional/language/add');
    $this->drupalPostForm(NULL, ['predefined_langcode' => 'de'], t('Add language'));

    // Create a vocabulary to which the terms will be assigned.
    $this->drupalGet('admin/structure/taxonomy/add');

    $edit = [
      'vid' => 'test_tax_vocab',
      'name' => 'Test',
    ];
    $this->drupalPostForm(NULL, $edit, t('Save'));

    $this->drupalGet('admin/structure/taxonomy/manage/test_tax_vocab');
    // Configure the vocabulary to not hide the language selector.
    $edit = [
      'default_language[language_alterable]' => TRUE,
      'default_language[langcode]' => 'en',
      'default_language[content_translation]' => TRUE,
      'langcode' => 'en',
    ];
    $this->drupalPostForm('admin/structure/taxonomy/manage/test_tax_vocab', $edit, t('Save'));

    // Add a first term in language 'en'.
    $this->drupalGet('admin/structure/taxonomy/manage/test_tax_vocab/add');
    $term_edit = [
      'name[0][value]' => 'Fruit',
      'langcode[0][value]' => 'en',
    ];
    $this->drupalPostForm(NULL, $term_edit, t('Save'));
    $terms = taxonomy_term_load_multiple_by_name($term_edit['name[0][value]']);
    $term = reset($terms);

    // Add the German translation to the term.
    $term_edit = [
      'name[0][value]' => 'Obst',
    ];

    $this->drupalGet('taxonomy/term/' . $term->id() . '/translations/add/en/de');
    $this->drupalPostForm(NULL, $term_edit, t('Save'));

    // Create a testing menu.
    $this->drupalGet('admin/structure/menu/add');
    $edit = [
      'id' => 'test-menu',
      'label' => 'Test',
      'langcode' => 'en',
    ];
    $this->drupalPostForm(NULL, $edit, t('Save'));

    // Create new taxonomy menu.
    $this->drupalGet('admin/structure/taxonomy_menu/add');
    $edit = [
      'id' => 'test_tax_menu',
      'label' => 'test tax menu',
      'vocabulary' => 'test_tax_vocab',
      'menu' => 'test-menu',
      'expanded' => 1,
      'depth' => '1',
      'menu_parent' => 'test-menu:',
    ];
    $this->drupalPostForm(NULL, $edit, t('Save'));
  }

  /**
   * Test for the English taxonomy term in the menu.
   *
   * Test whether the English taxonomy term 'Fruit' is in the English
   * version of the 'test-menu' menu.
   */
  public function testTaxonomyMenuLanguageEnglishTitle() {
    $this->drupalGet('admin/structure/menu/manage/test-menu');
    $this->assertSession()->linkExists('Fruit');
  }

  /**
   * Test for the German translated taxonomy term in the menu.
   *
   * Test whether the German translation, 'Obst', of the English taxonomy term
   * 'Fruit' is in the German version of the 'test-menu' menu.
   */
  public function testTaxonomyMenuLanguageGermanTitle() {
    $this->drupalGet('de/admin/structure/menu/manage/test-menu');
    $this->assertSession()->linkExists('Obst');
  }

}
