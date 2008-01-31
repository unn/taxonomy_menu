TAXONOMY MENU


INTRO
=====

This module adds links to taxonomy terms to the global navigation menu.


INSTALLATION
============

1) Place this module directory into your Drupal modules directory.

2) Enable the taxonomy_menu module in Drupal, at:
   administration -> site configuration -> modules (admin/build/modules)

3) Choose which vocabularies to appear in the menu at:
   administration -> site configuration -> modules -> taxonomy menu
   (admin/settings/taxonomy_menu)


INTEGRATE WITH VIEWS
====================

1) Install Views module

2) Enable Views and Views UI module

3) Create a view normally. You can display your content on a table,
   only the teaser, the full content; you decide.

4) Yet on Views page, on Argument fieldset, include Vocabulary ID as
   the first argument and Term ID as the second (both optional).

5) On Taxonomy Menu Settings page, select the new Views that you
   created.
