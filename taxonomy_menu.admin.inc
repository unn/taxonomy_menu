<?php

/**
 * @file
 * Administrative page callbacks for the Taxonomy menu module.
 */

/**
 * Form constructor for the vocabulary editing form. We add our taxonomy_menu
 * settings in here on a per-vocabulary basis.
 *
 * @see taxonomy_menu_vocab_submit()
 */
function taxonomy_menu_form_taxonomy_form_vocabulary(&$form, &$form_state) {
  // Do not alter on deletion.
  if (isset($form_state['confirm_delete']) && isset($form_state['values']['vid'])) {
    return;
  }

  // Build options.
  $defaults = taxonomy_menu_taxonomy_menu_vocabulary_settings();
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
    '#description' => t('<b>Warning:</b> Multi-terms path is not available in Drupal by default, which means that you will have to register it using a module like Views for example.'),
    '#default_value' => taxonomy_menu_variable_get('path', $vid, 0),
    '#options' => taxonomy_menu_get_paths(),
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

  // Path options.
  $form['taxonomy_menu']['options_paths'] = array(
    '#type' => 'fieldset',
    '#title' => t('Path options'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  /*$form['taxonomy_menu']['options_paths']['display_descendants'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display descendants'),
    '#description' => t('Path will be changed to taxonomy/term/tid+tid+tid... for all terms that have child terms.'),
    '#default_value' => taxonomy_menu_variable_get('display_descendants', $vid, $defaults['display_descendants']),
  );*/
  $form['taxonomy_menu']['options_paths']['end_all'] = array(
    '#type' => 'checkbox',
    '#title' => t("Use 'all' at the end of URL"),
    '#description' => t('This changes tid+tid+tid to "All" in term when <em>Display descendants</em> has been selected.<br />Only used if <em>Menu path type</em> is "Default path".<br />Works with default taxonomy page.'),
    '#default_value' => taxonomy_menu_variable_get('end_all', $vid, $defaults['end_all']),
    '#disabled' => TRUE,
  );

  // Other options.
  $form['taxonomy_menu']['options_structure'] = array(
    '#type' => 'fieldset',
    '#title' => t('Structure options'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['taxonomy_menu']['options_structure']['expanded'] = array(
    '#type' => 'checkbox',
    '#title' => t('Auto expand menu items'),
    '#description' => t('Automatically show all menu items as expanded.'),
    '#default_value' => taxonomy_menu_variable_get('expanded', $vid, $defaults['expanded']),
  );
  $form['taxonomy_menu']['options_structure']['hide_empty_terms'] = array(
    '#type' => 'checkbox',
    '#title' => t('Hide empty terms'),
    '#description' => t('Hide terms with no items attached to them. This is not recursive by default.'),
    '#default_value' => taxonomy_menu_variable_get('hide_empty_terms', $vid, $defaults['hide_empty_terms']),
  );
  $form['taxonomy_menu']['options_structure']['flat'] = array(
    '#type' => 'checkbox',
    '#title' => t('Flatten the taxonomy\'s hierarchy in the menu'),
    '#description' => t('Add all menu items to the same level rather than hierarchically.'),
    '#default_value' => taxonomy_menu_variable_get('flat', $vid, $defaults['flat']),
  );

  $form['taxonomy_menu']['options_markup'] = array(
    '#type' => 'fieldset',
    '#title' => t('Markup options'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['taxonomy_menu']['options_markup']['display_title_attr'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display HTML title attributes on menu links.'),
    '#description' => t('Extra information, like the description of the term, is shown as a tooltip text when the mouse moves over the menu link.'),
    '#default_value' => taxonomy_menu_variable_get('display_title_attr', $vid, $defaults['display_title_attr']),
  );
  $form['taxonomy_menu']['options_markup']['term_item_description'] = array(
    '#type' => 'checkbox',
    '#title' => t('HTML title: Use description of the taxonomy term.'),
    '#description' => t('Title will default to empty if the description of the term is not used.'),
    '#default_value' => taxonomy_menu_variable_get('term_item_description', $vid, $defaults['term_item_description']),
    '#states' => array(
      'visible' => array(
        ':input[name="taxonomy_menu[options_markup][display_title_attr]"]' => array('checked' => TRUE),
      ),
    ),
  );
  $form['taxonomy_menu']['options_markup']['display_num'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display the count of nodes referencing a term in the menu link title.'),
    '#description' => t('Number of nodes is calculated the Drupal Core way. This is not recursive by default.'),
    '#default_value' => taxonomy_menu_variable_get('display_num', $vid, $defaults['display_num']),
  );
  /*$form['taxonomy_menu']['options_markup']['voc_item_description'] = array(
    '#type' => 'checkbox',
    '#title' => t('Add description for vocabulary'),
    '#description' => t('Add the vocabulary description to the vocabulary menu item.'),
    '#default_value' => taxonomy_menu_variable_get('voc_item_description', $vid, $defaults['voc_item_description']),
  );*/

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
  $vid = $form_state['values']['vid'];
  // Flatten array of submitted settings so we can save them more easily.
  $flatten_settings = _taxonomy_menu_flatten_form_settings($form_state['values']['taxonomy_menu']);

  // If menu location has been set to disabled, don't throw notices by trying to
  // explode 0 with ':' .
  $vocab_parent =  $flatten_settings['vocab_parent'];
  $menu_location = ($vocab_parent == '0') ? '0:0' : $vocab_parent;
  list($flatten_settings['vocab_menu'], $flatten_settings['vocab_parent']) = explode(':', $menu_location);

  // Get all the settings that have changed since the last submit. If some of
  // them have changed, then update the taxonomy menu.
  $changed_settings = array();
  if ($vid != 0) {
    $changed_settings = _taxonomy_menu_get_changed_settings($flatten_settings, $vid);
    if (!empty($changed_settings)) {
      $update = TRUE;
      // Options have changed, save/update the menu.
      $menu_change = in_array('vocab_parent', $changed_settings) || in_array('vocab_menu', $changed_settings);
      if ($menu_change) {
        // Menu location has changed.
        if ($menu_disabled) {
          // Menu was disabled, delete all existing menu links.
          taxonomy_menu_menu_links_delete($vid);
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
    elseif (!$flatten_settings['rebuild']) {
      // Display a notification message. Nothing to update.
      drupal_set_message(t('The Taxonomy menu was not updated. Nothing to update.'), 'status');
    }
  }

  // Save all the submitted values.
  _taxonomy_menu_save_form_settings($flatten_settings, $vid);

  // We don't need to check for the disabled menu location because the rebuild
  // function will delete the taxonomy menu in all cases.
  if ($flatten_settings['rebuild']) {
    taxonomy_menu_rebuild($vid);
  }
  elseif ($insert) {
    // Update only menu links that are available in taxonomy_menu table.
    taxonomy_menu_menu_links_insert($vid);
  }
  elseif ($update) {
    // Update only menu links that are available in taxonomy_menu table.
    taxonomy_menu_menu_links_update($vid);
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
        taxonomy_menu_menu_links_update($vid);
      }
    }
  }
}

/**
 * Get all the taxonomy vocabulary settings available in modules implementing
 * the hook.
 *
 * This is useful in order to process the different settings altogether.
 *
 * @return array
 *   An array of available Taxonomy Menu settings and their respective defaults.
 */
function taxonomy_menu_get_vocabulary_settings() {
  return module_invoke_all('taxonomy_menu_vocabulary_settings');
}

/**
 * Implements hook_taxonomy_menu_vocabulary_settings()
 */
function taxonomy_menu_taxonomy_menu_vocabulary_settings() {
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
    //'display_descendants' => FALSE,
    'end_all' => FALSE,
    'display_title_attr' => FALSE,
  );

  return $defaults;
}

/**
 * Helper function to find which submitted values have changed upon submission
 * of a vocabulary's creation/updating form.
 *
 * @param array $submitted_settings
 *   The submitted taxonomy menu settings, includes values that are not in the
 *   'extended options' fieldset like path or menu location.
 * @param int $vid
 *   The vid of the vocabulary.
 *
 * @return $changed_settings
 *   an array of options, which have changed since the last submit.
 */
function _taxonomy_menu_get_changed_settings($submitted_settings, $vid) {
  $saved_settings = array();
  $defaults = taxonomy_menu_get_vocabulary_settings();
  // Build an array of all saved values of settings from taxonomy menu and
  // other modules.
  foreach ($submitted_settings as $key => $option) {
    $value = taxonomy_menu_variable_get($key, $vid, $defaults[$key]);
    $saved_settings[$key] = $value;
  }
  // Keep only the values that changed.
  $changed_settings = array_keys(array_diff_assoc($saved_settings, $submitted_settings));

  return $changed_settings;
}

/**
 * Helper function, which recursively saves all the submitted settings values
 * of a form.
 *
 * @param $settings
 *   An array of all the settings' values to be saved.
 */
function _taxonomy_menu_save_form_settings($settings, $vid) {
  foreach ($settings as $key => $value) {
    if (is_array($value)) {
      _taxonomy_menu_save_form_settings($value, $vid);
    }
    else {
      taxonomy_menu_variable_set($key, $vid, $value);
    }
  }
}

/**
 * Flatten an array of submitted values.
 *
 * @param array $settings
 *   An array of settings to be flattened.
 *
 * @return array
 *   A flattened array of settings.
 *
 * @TODO Compatible with PHP 5.3 and more only. Find a way for 5.2 compatibility.
 */
function _taxonomy_menu_flatten_form_settings($settings) {
  $flatten = array();
  array_walk_recursive($settings, function($a, $b) use (&$flatten) { $flatten[$b] = $a; });

  return $flatten;
}

/**
 * Checks a path exists.
 *
 * @param string $path
 *   The path to check.
 *
 * @return
 *   TRUE if it is a valid path, FALSE otherwise.
 *
 * Directly inspired by drupal_valid_path function from core but this function
 * throws notices. cf. http://drupal.org/node/876580
 * @TODO Use the core drupal_valid_path function instead when fixed.
 */
function _taxonomy_menu_valid_path($path) {
  return db_query("SELECT * FROM {menu_router} where path = :path", array(':path' => $path))->fetchAssoc();
}
