<?php

namespace Drupal\taxonomy_menu\Tests\Unit\Plugin\Menu;

use Drupal\Core\Entity\EntityManager;
use Drupal\Core\Menu\StaticMenuLinkOverridesInterface;
use Drupal\taxonomy\TermInterface;
use Drupal\taxonomy\TermStorageInterface;
use Drupal\taxonomy_menu\Plugin\Menu\TaxonomyMenuMenuLink;
use Drupal\Tests\UnitTestCase;

/**
 *
 * @covers TaxonomyMenuMenuLink
 *
 * @group taxonomy_menu
 *
 * @property array configuration
 * @property string plugin_id
 * @property array plugin_definition
 * @property \Prophecy\Prophecy\ObjectProphecy term_storage
 * @property \Prophecy\Prophecy\ObjectProphecy entity_repository
 * @property \Prophecy\Prophecy\ObjectProphecy static_overrides
 */
class TaxonomyMenuMenuLinkTest extends UnitTestCase {

  public function setUp()
  {
    parent::setUp();

    $this->configuration = [];
    $this->plugin_id = 'taxonomy_menu.menu_link:taxonomy_menu.menu_link.categories.1';
    $this->plugin_definition = ['metadata' => ['taxonomy_term_id' => '1']];
    $this->term_storage = $this->prophesize(TermStorageInterface::class);
    $this->entity_repository = $this
      ->prophesize(EntityManager::class);
    $this->static_overrides = $this
      ->prophesize(StaticMenuLinkOverridesInterface::class);

    $this->subject = new TaxonomyMenuMenuLink(
      $this->configuration,
      $this->plugin_id,
      $this->plugin_definition,
      //$this->term_storage->reveal(),
      $this->entity_repository->reveal(),
      $this->static_overrides->reveal()
    );
  }

  /**
   * Test that the menu link titles and description are translated
   */
  public function testTranslatedMenuLink() {
    // Arrange
    $original = $this->prophesize(TermInterface::class);
    $original->label()->willReturn('Original title');
    $original->getDescription()->willReturn('Original description');

    $translated = $this->prophesize(TermInterface::class);
    $translated->label()->willReturn('Translated title');
    $translated->getDescription()->willReturn('Translated description');

    $this->term_storage->load('1')->willReturn($original->reveal());
    $this->entity_repository->getTranslationFromContext($original->reveal())
      ->willReturn($translated->reveal());

    // Act
    $title = $this->subject->getTitle();
    $description = $this->subject->getDescription();

    // Assert
    $this->assertEquals('Translated title', $title);
    $this->assertEquals('Translated description', $description);
  }
}
