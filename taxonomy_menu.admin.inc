<?php

/**
 * @file
 * Administrative page callbacks for the Taxonomy menu module.
 */

/**
 * Form constructor for the vocabulary editing form. We add our taxonomy_menu
 * options in here on a per-vocabulary basis.
 *
 * @see taxonomy_menu_vocab_submit()
 */
function taxonomy_menu_form_taxonomy_form_vocabulary(&$form, &$form_state) {
  // Do not alter on deletion.
  if (isset($form_state['confirm_delete']) && isset($form_state['values']['vid'])) {
    return;
  }

  // Build options.
  $defaults = _taxonomy_menu_get_form_settings_defaults();
  $form['taxonomy_menu'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#title' => t('Taxonomy menu'),
    '#weight' => 10,
    '#tree' => TRUE,
  );

  // Get the vocabulary ID.
  $vid = (isset($form['vid']) && $form['vid']['#value']) ? $form['vid']['#value'] : 0;

  // Turn the vocab terms into menu items and add an option at the top for
  // disabling the menu.
  $menu_items = menu_parent_options(menu_get_menus(), array('mlid' => 0));
  array_unshift($menu_items, '= NONE =');
  // Try to get the current menu location value if the vocabulary has already
  // been created.
  if ($vid) {
    $menu_name = taxonomy_menu_variable_get('vocab_menu', $vid, 0);
    $mlid = taxonomy_menu_variable_get('vocab_parent', $vid, 0);
    $current_menu_value =  $menu_name . ':' . $mlid;
    $default_menu = isset($menu_items[$current_menu_value]) ? $current_menu_value : 0;
  }
  else {
    $default_menu = 0;
  }

  // Menu location.
  $form['taxonomy_menu']['vocab_parent'] = array(
    '#type' => 'select',
    '#title' => t('Menu location'),
    '#default_value' => $default_menu,
    '#options' => $menu_items,
    '#description' => t('The menu and parent under which to insert taxonomy menu items.'),
    '#attributes' => array('class' => array('menu-title-select')),
  );
  // Path.
  $form['taxonomy_menu']['path'] = array(
    '#type' => 'select',
    '#title' => t('Menu path type'),
    '#description' => t('The path will be taxonomy/term/tid if <em>Default</em> has been selected.<br />The menu path will be passed through drupal_get_path_alias() function so all aliases will be applied.'),
    '#default_value' => taxonomy_menu_variable_get('path', $vid, 0),
    '#options' => _taxonomy_menu_get_paths(),
  );
  // Sync
  $variable_name = _taxonomy_menu_build_variable('sync', $vid);
  $form['taxonomy_menu']['sync'] = array(
    '#type' => 'checkbox',
    '#title' => t('Synchronise changes to this vocabulary'),
    '#description' => t('Every time a term is added/deleted/modified, the corresponding menu link will be altered too.'),
    '#default_value' => taxonomy_menu_variable_get('sync', $vid, $defaults['sync']),
  );
  // Rebuild
  $variable_name = _taxonomy_menu_build_variable('rebuild', $vid);
  $form['taxonomy_menu']['rebuild'] = array(
    '#type' => 'checkbox',
    '#title' => t('Rebuild the menu on submit.'),
    '#description' => t('<strong>Warning</strong>: This will delete then re-create all of the menu items. Only use this option if you are experiencing issues like missing menu items or other inconsistencies.'),
    '#default_value' => taxonomy_menu_variable_get('rebuild', $vid, $defaults['rebuild']),
  );

  // Other options.
  $form['taxonomy_menu']['options'] = array(
    '#type' => 'fieldset',
    '#title' => t('Extended options'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['taxonomy_menu']['options']['expanded'] = array(
    '#type' => 'checkbox',
    '#title' => t('Auto expand menu items'),
    '#description' => t('Automatically show all menu items as expanded.'),
    '#default_value' => taxonomy_menu_variable_get('expanded', $vid, $defaults['expanded']),
  );
  $form['taxonomy_menu']['options']['display_title_attr'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display HTML title attributes on menu links.'),
    '#description' => t('Extra information, like the description of the term, is shown as a tooltip text when the mouse moves over the menu link.'),
    '#default_value' => taxonomy_menu_variable_get('display_title_attr', $vid, $defaults['display_title_attr']),
  );
  $form['taxonomy_menu']['options']['term_item_description'] = array(
    '#type' => 'checkbox',
    '#title' => t('Add description for terms'),
    '#description' => t('Add the term description to the term menu item.'),
    '#default_value' => taxonomy_menu_variable_get('term_item_description', $vid, $defaults['term_item_description']),
  );
  $form['taxonomy_menu']['options']['display_num'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display number of items'),
    '#description' => t('Display the number of items per taxonomy terms. Will not show up for vocabulary menu items.'),
    '#default_value' => taxonomy_menu_variable_get('display_num', $vid, $defaults['display_num']),
  );
  $form['taxonomy_menu']['options']['hide_empty_terms'] = array(
    '#type' => 'checkbox',
    '#title' => t('Hide empty terms'),
    '#description' => t('Hide terms with no items attached to them.'),
    '#default_value' => taxonomy_menu_variable_get('hide_empty_terms', $vid, $defaults['hide_empty_terms']),
  );
  $form['taxonomy_menu']['options']['voc_item_description'] = array(
    '#title' => t('Add description for vocabulary'),
    '#description' => t('Add the vocabulary description to the vocabulary menu item.'),
    '#default_value' => taxonomy_menu_variable_get('voc_item_description', $vid, $defaults['voc_item_description']),
  );
  $form['taxonomy_menu']['options']['flat'] = array(
    '#type' => 'checkbox',
    '#title' => t('Flatten the taxonomy\'s hierarchy in the menu'),
    '#description' => t('Add all menu items to the same level rather than hierarchically.'),
    '#default_value' => taxonomy_menu_variable_get('flat', $vid, $defaults['flat']),
  );

  // Disabled / Not tested options
  $form['taxonomy_menu']['options']['voc_item'] = array(
    '#type' => 'checkbox',
    '#title' => t('Add item for vocabulary'),
    '#description' => t('Shows the vocabulary name as the top level menu item of the taxonomy menu.'),
    '#disabled' => TRUE,
    '#default_value' => taxonomy_menu_variable_get('voc_item', $vid, $defaults['voc_item']),
  );
  $form['taxonomy_menu']['options']['voc_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Custom name for vocabulary item'),
    '#description' => t('Changes the name of the vocabulary item (if enabled above). Leave blank to use the name of the vocabulary.'),
    '#disabled' => TRUE,
    '#default_value' => taxonomy_menu_variable_get('voc_name', $vid, $defaults['voc_name']),
  );
  $form['taxonomy_menu']['options']['display_descendants'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display descendants'),
    '#description' => t('Changes the default path to taxonomy/term/tid+tid+tid for all terms thave have child terms.'),
    '#default_value' => taxonomy_menu_variable_get('display_descendants', $vid, $defaults['display_descendants']),
    '#disabled' => TRUE,
  );
  $form['taxonomy_menu']['options']['end_all'] = array(
    '#type' => 'checkbox',
    '#title' => t("Use 'all' at the end of URL"),
    '#description' => t('This changes tid+tid+tid to "All" in term when <em>Display descendants</em> has been selected.<br />Only used if <em>Menu path type</em> is "Default path".<br />Works with default taxonomy page.'),
    '#disabled' => TRUE,
    '#default_value' => taxonomy_menu_variable_get('end_all', $vid, $defaults['end_all']),
  );

  // Move the buttons to the bottom of the form.
  $form['submit']['#weight'] = 49;
  $form['delete']['#weight'] = 50;

  // Add an extra submit handler to save these settings.
  $form['#submit'][] = 'taxonomy_menu_vocab_submit';
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
  // Initialize flag variables for updating/rebuilding the taxonomy menu.
  $update = FALSE;
  $insert = FALSE;
  $menu_disabled = $form_state['values']['taxonomy_menu']['vocab_parent'] == '0';
  // Get the vid of the submitted vocabulary.
  $vid = $form_state['values']['vid'];

  // Submitted taxonomy menu's options. Split the menu location into menu name
  // and mlid to save menu and parent separately.
  // Options array is flattened.
  $flatten_options = _taxonomy_menu_flatten_form_settings($form_state['values']['taxonomy_menu']);

  // If menu location has been set to disabled, don't throw notices by trying to
  // explode 0 with ':' .
  $vocab_parent =  $flatten_options['vocab_parent'];
  $menu_location = ($vocab_parent == '0') ? '0:0' : $vocab_parent;
  list($flatten_options['vocab_menu'], $flatten_options['vocab_parent']) = explode(':', $menu_location);

  // Get all the options that changed since last submit. If some values have
  // changed, then update the taxonomy menu.
  $changed_options = array();
  if ($vid != 0) {
    $changed_options = _taxonomy_menu_get_changed_options($flatten_options, $vid);
    if (!empty($changed_options)) {
      $update = TRUE;
      // Options have changed, save/update the menu.
      $menu_change = in_array('vocab_parent', $changed_options) || in_array('vocab_menu', $changed_options);
      if ($menu_change) {
        // Menu location has changed.
        if ($menu_disabled) {
          // Menu was disabled, delete all existing menu links.
          _taxonomy_menu_delete_menu_links($vid);
        }
        else {
          // Menu location has been changed and is not disabled.
          $old_vocab_parent = taxonomy_menu_variable_get('vocab_parent', $vid, '0');
          $old_vocab_menu = taxonomy_menu_variable_get('vocab_menu', $vid, '0');
          if ($old_vocab_menu == '0' && $old_vocab_parent == '0') {
            // Menu was disabled before, create new links.
            $insert = TRUE;
          }
        }
        // Do a full menu rebuild in case we have removed or moved the menu.
        variable_set('menu_rebuild_needed', TRUE);
      }
    }
    elseif (!$flatten_options['rebuild']) {
      // Display a notification message. Nothing to update.
      drupal_set_message(t('The Taxonomy menu was not updated. Nothing to update.'), 'status');
    }
  }

  // Save all the submitted values.
  _taxonomy_menu_save_form_settings($flatten_options, $vid);

  // We don't need to check for the disabled menu location because the rebuild
  // function will delete the taxonomy menu in all cases.
  if ($flatten_options['rebuild']) {
    _taxonomy_menu_rebuild($vid);
  }
  elseif ($insert) {
    // Update only menu links that are available in taxonomy_menu table.
    _taxonomy_menu_insert_menu_links($vid);
  }
  elseif ($update) {
    // Update only menu links that are available in taxonomy_menu table.
    _taxonomy_menu_update_menu_links($vid);
  }
}

/**
 * Form constructor for the terms overview form. We provide an additional callback.
 *
 * Using hook_taxonomy_vocabulary_update is nicer then callback, but gives less
 * info and does not always fire.
 *
 * @see taxonomy_menu_overview_terms_submit()
 */
function taxonomy_menu_form_taxonomy_overview_terms(&$form, &$form_state) {
  $form['#submit'][] = 'taxonomy_menu_overview_terms_submit';
}

/**
 * Additional submit handler for terms overview form.
 *
 * @see taxonomy_menu_form_taxonomy_overview_terms()
 */
function taxonomy_menu_overview_terms_submit(&$form, &$form_state) {
  // This form has the following flow of buttons:
  // 1. [Save] --> update taxonomy menu
  // 2. [Reset to alphabetical] --> do nothing, wait for confirmation page
  // 3. [Reset to alphabetical][Reset to alphabetical] --> update taxonomy menu
  // 4. [Reset to alphabetical][Cancel] --> do nothing
  $update = FALSE;
  if (isset($form_state['confirm_reset_alphabetical']) && $form_state['confirm_reset_alphabetical'] === TRUE) {
    if ($form_state['values']['reset_alphabetical'] === TRUE) {
      $update = TRUE;
    }
  }
  else if ($form_state['clicked_button']['#value'] == t('Save')){
    $update = TRUE;
  }
  if ($update === TRUE) {
    $vid = isset($form['vid']['#value']) ? $form['vid']['#value'] : $form['#vocabulary']->vid;
    if ($vid) {
      $menu_name = taxonomy_menu_variable_get('vocab_menu', $vid, '0');
      $sync = taxonomy_menu_variable_get('sync', $vid, 0);
      if ($menu_name && $sync) {
        _taxonomy_menu_update_menu_links($vid);
      }
    }
  }
}

/**
 * Helper function to find which submitted values have changed upon submission
 * of a vocabulary's creation/updating form.
 *
 * @param $form
 *   the submitted taxonomy menu options, includes values that are not in the
 *   'extended options' fieldset like path or menu location.
 * @param
 *   the vid of the vocabulary.
 *
 * @return $changed_options
 *   an array of options, which have changed since the last submit.
 */
function _taxonomy_menu_get_changed_options($submitted_options, $vid) {
  $saved_options = array();
  $defaults = _taxonomy_menu_get_form_settings_defaults();
  // Build an array of all saved values of options from taxonomy menu and
  // other modules.
  foreach ($submitted_options as $key => $option) {
    $value = taxonomy_menu_variable_get($key, $vid, $defaults[$key]);
    $saved_options[$key] = $value;
  }

  // Keep only the values that changed.
  $changed_options = array_keys(array_diff_assoc($saved_options, $submitted_options));

  return $changed_options;
}

/**
 * Flatten an array of submitted values.
 *
 * @param $options
 *   An array of options to be flattened.
 *
 * @return
 *   An flattened array of options.
 *
 * @TODO Compatible with PHP 5.3 and more only. Find a way for 5.2 compatibility.
 */
function _taxonomy_menu_flatten_form_settings($options) {
  $flatten = array();
  array_walk_recursive($options, function($a, $b) use (&$flatten) { $flatten[$b] = $a; });

  return $flatten;
}

/**
 * Recursively saves all the submitted values of a form.
 *
 * @param $options
 *   An array of all the settings to be saved.
 */
function _taxonomy_menu_save_form_settings($options, $vid) {
  foreach ($options as $key => $value) {
    if (is_array($value)) {
      _taxonomy_menu_save_form_settings($value, $vid);
    }
    else {
      taxonomy_menu_variable_set($key, $vid, $value);
    }
  }
}

/**
 * Get default values for Taxonomy menu options.
 * This is useful in order to check settings that have changed.
 *
 * @see _taxonomy_menu_get_changed_options()
 */
function _taxonomy_menu_get_form_settings_defaults() {
  $defaults = array(
    'vocab_parent' => 0,
    'vocab_menu' => 0,
    'path' => 0,
    'sync' => TRUE,
    'rebuild' => FALSE,
    'expanded' => TRUE,
    'term_item_description' => FALSE,
    'display_num' => FALSE,
    'hide_empty_terms' => FALSE,
    'flat' => FALSE,
    'voc_item_description' => FALSE,
    'voc_item' => FALSE,
    'voc_name' => '',
    'display_descendants' => FALSE,
    'end_all' => FALSE,
    'display_title_attr' => FALSE,
  );

  return $defaults;
}
