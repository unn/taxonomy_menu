adv_taxonomy_menu

This is a modification of the taxonomy_menu module which creates the menu from any number of single level vocabularies, which is useful in situations where each category shares the same subcategories.  Take for instance a heirarchy like clothing.  The levels might be as follows:

vocab 1: Autumn, Summer, Fall
vocab 2: men, women
vocab 3: shirts, pants, shoes
vocab 4: size 10, 12, 14
vocab 5: colour red, green yellow

To create this heirarchy as a fixed system using taxonomy would be tiresome and error prone.  Instead the adv_taxonomy_menu enables you to set the the vocabulary to use at each level of the menu and creates the menu system from your directions.  Furthermore you can create any number of such menus on the same site.   It produces normal html code which can be themed using any standard css styling.

Install

Install module and go to admin/settings/adv_taxonomy_menu. Add a menu and enter details.  The menu is created from 2 or more vocabularies.  Choose the vocabularies by nominating the level at which you want the vocabulary to apply in the menu.  1 means the vocabulary will be on top.  Choose also the name that you wish to appear in the url before the terms eg: mystuff/1/11/111.  Currently only the default view is available.

Developer Notes

The adv_taxonomy_menu engine can be used by other modules to create menu systems on the fly by invoking hook_adv_taxononmy_menu_sql_alter.  The hook function should be a modification of taxonomy_select_nodes().
