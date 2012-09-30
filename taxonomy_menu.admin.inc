<?php

/**
 * @file
 * Administrative page callbacks for the Taxonomy Menu module.
 */

/**
 * Form constructor for the vocabulary editing form. We add our taxonomy_menu
 * options in here on a per-vocab basis.
 *
 * @see taxonomy_menu_vocab_submit()
 */
function taxonomy_menu_form_taxonomy_form_vocabulary(&$form, &$form_state) {
  // Do not alter on deletion.
  if (isset($form_state['confirm_delete']) && isset($form_state['values']['vid'])) {
    return;
  }
  // Choose a menu to add link items to.
  $menus = menu_get_menus();
  array_unshift($menus, t('- Disabled -'));

  // Options for path if tokens are not enabled.
  $paths = _taxonomy_menu_get_paths();

  $form['taxonomy_menu'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#title' => t('Taxonomy menu'),
    '#weight' => 10,
    '#tree' => TRUE,
  );
  // This turns the vocab terms into menu items.
  $item['mlid'] = 0;
  $menu_items = menu_parent_options(menu_get_menus(), $item);
  array_unshift($menu_items, '= DISABLED =');

  // The vid isn't set when a new vocabulary is being created.
  if (isset($form['vid']['#value'])) {
    $default = variable_get(_taxonomy_menu_build_variable('vocab_menu', $form['vid']['#value']), NULL) . ':' .
        variable_get(_taxonomy_menu_build_variable('vocab_parent', $form['vid']['#value']), NULL);
    if (!isset($menu_items[$default])) {
      $default = 0;
    }
  }
  else {
    $default = 0;
  }

  $form['taxonomy_menu']['vocab_parent'] = array(
    '#type' => 'select',
    '#title' => t('Menu location'),
    '#default_value' => $default,
    '#options' => $menu_items,
    '#description' => t('The menu and parent under which to insert taxonomy menu items.'),
    '#attributes' => array('class' => array('menu-title-select')),
  );

  $form['taxonomy_menu']['path'] = array(
    '#type' => 'select',
    '#title' => t('Menu path type'),
    '#default_value' => isset($form['vid']['#value']) ? variable_get(_taxonomy_menu_build_variable('path', $form['vid']['#value']), 0) : 0,
    '#options' => $paths,
    '#description' => t('The path will be taxonomy/term/tid if <em>Default</em> has been selected.<br />The menu path will be passed through drupal_get_path_alias() function so all aliases will be applied.'),
  );

  // Get taxonomy menu form options.
  if (isset($form['vid']) && $form['vid']['#value']) {
    $vid = $form['vid']['#value'];
  }
  else {
    $vid = 0;
  }
  $form['taxonomy_menu']['options'] = _taxonomy_menu_create_options($vid);

  // Rebuild the menu.
  $form['taxonomy_menu']['options']['rebuild'] = array(
    '#type' => 'checkbox',
    '#title' => t('Select to rebuild the menu on submit.'),
    '#default_value' => 0,
    '#weight' => 20,
    '#description' => t('Rebuild the menu on submit. <strong>Warning</strong>: This will delete then re-create all of the menu items. Only use this option if you are experiencing issues like missing menu items or other inconsistencies.'),
  );
  // Move the buttons to the bottom of the form.
  $form['submit']['#weight'] = 49;
  $form['delete']['#weight'] = 50;

  // Add an extra submit handler to save these settings.
  $form['#submit'][] = 'taxonomy_menu_vocab_submit';
}

/**
 * Form constructor for the terms overview form.
 * We provide an additional callback.
 *
 * @see taxonomy_menu_overview_submit()
 */
function taxonomy_menu_form_taxonomy_overview_terms(&$form, &$form_state) {
  // Add an extra submit handler to sync the rearranged terms with menu
  // @ TODO: using hook_taxonomy_vocabulary_update is nicer then callback,
  // but gives less info and does not always fire.
  $form['#submit'][] = 'taxonomy_menu_overview_submit';
}

/**
 * Form submission handler for taxonomy_form_vocabulary().
 *
 * Check to see if the user has selected a different menu, and only rebuild
 * if this is the case.
 *
 * @see taxonomy_menu_form_taxonomy_form_vocabulary()
 */
function taxonomy_menu_vocab_submit($form, &$form_state) {
  $vid = $form_state['values']['vid'];
  $changed = FALSE;

  if (is_numeric($form_state['values']['taxonomy_menu']['vocab_parent'])) {
    // Menu location has been set to disabled, don't want to throw notices.
    $form_state['values']['taxonomy_menu']['vocab_parent'] = '0:0';
  }

  // Split the menu location into menu name and menu item id.
  list($vocab_parent['vocab_menu'], $vocab_parent['vocab_parent']) = explode(':', $form_state['values']['taxonomy_menu']['vocab_parent']);

  // Init flag variables to avoid notices if changes haven't happened.
  $changed_menu = FALSE;
  $change_vocab_item = FALSE;
  $changed_path = FALSE;

  // Set the menu name and check for changes.
  $variable_name = _taxonomy_menu_build_variable('vocab_menu', $vid);
  if (_taxonomy_menu_check_variable($variable_name, $vocab_parent['vocab_menu'])) {
    $changed_menu = TRUE;
  }
  variable_set($variable_name, $vocab_parent['vocab_menu']);

  // Set the menu parent item and check for changes.
  $variable_name = _taxonomy_menu_build_variable('vocab_parent', $vid);
  if (_taxonomy_menu_check_variable($variable_name, $vocab_parent['vocab_parent'])) {
    $changed_menu = TRUE;
  }
  variable_set($variable_name, $vocab_parent['vocab_parent']);

  // Set the path and check for changes.
  $variable_name = _taxonomy_menu_build_variable('path', $vid);
  if (_taxonomy_menu_check_variable($variable_name, $form_state['values']['taxonomy_menu']['path'])) {
    $changed_path = TRUE;
  }
  variable_set($variable_name, $form_state['values']['taxonomy_menu']['path']);

  foreach ($form_state['values']['taxonomy_menu']['options'] as $key => $value) {
    // Create the variable name.
    $variable_name = _taxonomy_menu_build_variable($key, $vid);

    // Check to see if the vocab enable options has changed.
    if ($key == 'voc_item') {
      if (_taxonomy_menu_check_variable($variable_name, $value)) {
        $change_vocab_item = TRUE;
      }
    }

    // If $changed is alreayd set to true, then don't bother checking any others.
    if (!$changed) {
      // Check to see if the variable has changed.
      if (_taxonomy_menu_check_variable($variable_name, $value)) {
        $changed = TRUE;
      }
    }
    // Save variable.
    variable_set($variable_name, $value);
  }

  // If the menu hasn't changed and the menu is disabled then do not do anything else.
  if ($form_state['values']['taxonomy_menu']['options']['rebuild'] ||
      $changed_menu ||
      (!$changed_menu && variable_get(_taxonomy_menu_build_variable('vocab_menu', $vid), FALSE) == 0)) {
    // Rebuild if rebuild is selected, menu has changed or vocabulary option changed.
    if ($form_state['values']['taxonomy_menu']['options']['rebuild'] || $changed_menu || $change_vocab_item || $changed_path) {
      $message = _taxonomy_menu_rebuild($vid);
    }
    // If setting has changed and a menu item is enabled, then update all of the menu items.
    elseif ($changed && variable_get(_taxonomy_menu_build_variable('vocab_menu', $vid), FALSE)) {
      $message = _taxonomy_menu_update_link_items($vid);
    }

    // Do a full menu rebuild in case we have removed the menu or moved it between menus.
    variable_set('menu_rebuild_needed', TRUE);
    // Only send a message if one has been created.
    if (isset($message) && $message) {
      // $message is sanitized coming out of its source function,
      // no need to reclean it here
      drupal_set_message($message, 'status');
    }
  }
}

/**
 * Additional submit handler for terms overview form.
 *
 * @see taxonomy_menu_form_taxonomy_overview_terms()
 */
function taxonomy_menu_overview_submit(&$form, &$form_state) {
  // Only sync if taxonomy_menu is enabled for this vocab and the 'sync'
  // option has been checked.

  // This form has the following flow of buttons:
  // 1. [Save] --> rebuild taxonomy_menu
  // 2. [Reset to alphabetical] --> no rebuild yet
  // 3. [Reset to alphabetical][Reset to alphabetical] --> rebuild
  // 4. [Reset to alphabetical][Cancel] --> no rebuild
  // The code below avoids rebuilding after situation 2.

  if ($form_state['rebuild'] == FALSE && isset($form['#vocabulary']->vid) ) {
    // Try to catch the 'Save' button.
    $vid = $form['#vocabulary']->vid;
  }
  elseif ($form_state['rebuild'] == TRUE && isset($form['#vocabulary']->vid) ) {
    // Try to catch the 'Reset to alphabetical' button.
    $vid = NULL;
  }
  elseif ($form_state['rebuild'] == FALSE && isset($form['vid']['#value']) ) {
    // Try to catch the second (confirming) 'Reset to alphabetical' button.
    $vid = $form['vid']['#value'];
  }
  else {
    // The button [Reset to alphabetical] [Cancel] does not call this page.
    $vid = NULL;
  }

  if (isset($vid)) {
    $menu_name = variable_get(_taxonomy_menu_build_variable('vocab_menu', $vid), 0);
    $sync = variable_get(_taxonomy_menu_build_variable('sync', $vid), 0);
    if ($menu_name && $sync) {
      // Update all menu items (do not rebuild the menu).
      $message = _taxonomy_menu_update_link_items($vid);

      // Report status.
      if (isset($message)) {
        // Message is sanitized coming out of _taxonomy_menu_update_link_items.
        // No need to reclean it here.
        drupal_set_message($message, 'status');
      }

      // Rebuild the menu.
      menu_cache_clear($menu_name);
    }
  }
}

/**
 * Implements hook_taxonomy_menu_options().
 *
 * Returns Taxonomy Menu options for the vocabulary editing form.
 *
 * @return array
 *   An array of Taxonomy Menu options.
 *   Uses the value to set the variable taxonomy_menu_<value>_<machine_name>.
 *   The following values are available in the $otpions array:
 *     - #title - required.
 *     - #description - required.
 *     - #type - optional. The type of the form element.
 *     - #disabled - optional. Should the option be disabled or not.
 *     - default - optional. This is what will be used if the variable is not
 *     set. If empty, FALSE is used.
 */
function taxonomy_menu_taxonomy_menu_options() {
  $options = array();

  $options['sync'] = array(
    '#title' => t('Synchronise changes to this vocabulary'),
    '#description' => t('Every time a term is added/deleted/modified, the corresponding menu link will be altered too.'),
    'default' => TRUE,
  );

  $options['display_num'] = array(
    '#title' => t('Display number of items'),
    '#description' => t('Display the number of items per taxonomy terms. Will not show up for vocabulary menu items.'),
    'default' => FALSE,
  );

  $options['hide_empty_terms'] = array(
    '#title' => t('Hide empty terms'),
    '#description' => t('Hide terms with no items attached to them.'),
    'default' => FALSE,
  );

  $options['voc_item'] = array(
    '#title' => t('Add item for vocabulary'),
    '#description' => t('Shows the vocabulary name as the top level menu item of the taxonomy menu.'),
    '#disabled' => TRUE,
    'default' => FALSE,
  );

  $options['voc_item_description'] = array(
    '#title' => t('Add description for vocabulary'),
    '#description' => t('Add the vocabulary description to the vocabulary menu item.'),
    'default' => FALSE,
  );

  $options['term_item_description'] = array(
    '#title' => t('Add description for terms'),
    '#description' => t('Add the term description to the term menu item.'),
    'default' => FALSE,
  );

  $options['expanded'] = array(
    '#title' => t('Auto expand menu items'),
    '#description' => t('Automatically show all menu items as expanded.'),
    'default' => TRUE,
  );

  $options['flat'] = array(
    '#title' => t('Flatten the taxonomy\'s hierarchy in the menu'),
    '#description' => t('Add all menu items to the same level rather than hierarchically.'),
    'default' => FALSE,
  );

  $options['voc_name'] = array(
    '#title' => t('Custom name for vocabulary item'),
    '#description' => t('Changes the name of the vocabulary item (if enabled above). Leave blank to use the name of the vocabulary.'),
    '#type' => 'textfield',
    '#disabled' => TRUE,
    'default' => '',
  );

  $options['display_descendants'] = array(
    '#title' => t('Display descendants'),
    '#description' => t('Changes the default path to taxonomy/term/tid+tid+tid for all terms thave have child terms.'),
    'default' => FALSE,
  );

  $options['end_all'] = array(
    '#title' => t("Use 'all' at the end of URL"),
    '#description' => t('This changes tid+tid+tid to "All" in term when <em>Display descendants</em> has been selected.<br />Only used if <em>Menu path type</em> is "Default path".<br />Works with default taxonomy page.'),
    '#disabled' => TRUE,
    'default' => FALSE,
  );

  return $options;
}

/**
 * Creates Taxonomy Menu options.
 * Invokes hook_taxonomy_menu_options().
 *
 * @return $options array
 */
function _taxonomy_menu_create_options($vid) {
  $options = module_invoke_all('taxonomy_menu_options');

  // Cycle through field.
  foreach ($options as $field_name => $field_elements) {
    // Cycle through each value of the field.
    $variable_name = _taxonomy_menu_build_variable($field_name, $vid);

    // If the variable is set then use that, if the default key is set then use
    // that, otherwise use false.
    $options[$field_name]['#default_value'] =
    variable_get($variable_name,
        !empty($options[$field_name]['default']) ? $options[$field_name]['default'] : FALSE);

    // Set the type to checkbox if it is empty.
    if (empty($options[$field_name]['#type'])) {
      $options[$field_name]['#type'] = 'checkbox';
    }

    // Set the option feildset values.
    $options['#type'] = 'fieldset';
    $options['#title'] = t('Options');
    $options['#collapsible'] = TRUE;

    // Remove the default value from the array so we don't pass it to the form.
    unset($options[$field_name]['default']);
  }

  return $options;
}
