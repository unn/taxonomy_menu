<?php
// $Id$

/**
 * @author Jonathan Chaffer   <jchaffer@structureinteractive.com>
 * @author Bruno Massa        <http://drupal.org/user/67164>
 * @file taxonomy_menu.inc
 * It Generates menu links for all taxonomy terms
 */

/**
 * Admin area. Configure the module, setting which
 * vocabularies will be converted into menus items
 *
 * @return
 *   Array. The form fields.
 * @ingroup forms
 */
function _taxonomy_menu_admin(&$form_state) {
  $options = array(
    TAXONOMY_MENU_NONE    => t('No'),
    TAXONOMY_MENU_NORMAL  => t('Normal')
  );

  // If the Views module is enabled, add some special
  // new features
//   if (module_exists('views')) {
//     // Add the Views file with more functions
//     require_once(drupal_get_path('module', 'views') .'/views_cache.inc');
//
//     // Add a new options on Categories
//     $options[TAXONOMY_MENU_VIEW] = t('Views');
//
//     // Get the list of User generated views
//     $views = db_query("SELECT * FROM {view_view}");
//     while ($view = db_fetch_array($views)) {
//       $views_list[$view['name']] = $view['page_title'];
//     }
//
//     // Now get a list of default Views
//     foreach (_views_get_default_views() as $view => $viewdata) {
//       $views_list[$view] = $viewdata->name;
//     }
//   }

  // If the Taxonomy Default module is enabled, add some special
  // new features
  if (module_exists('taxonomy_defaults')) {
    $options[TAXONOMY_MENU_DEFAULT_TAX] = t('Module types');
  }

  // Create some options for each of the vocabularies
  foreach (taxonomy_get_vocabularies() as $vocab) {
    $form[$vocab->vid] = array(
      '#title'          => $vocab->name,
      '#tree'           => FALSE,
      '#type'           => 'fieldset'
    );
    $form[$vocab->vid]['taxonomy_menu_show_'. $vocab->vid] = array(
      '#default_value'  => variable_get('taxonomy_menu_show_'. $vocab->vid, TAXONOMY_MENU_NONE),
      '#options'        => $options,
      '#title'          => t('Show this category in menu'),
      '#type'           => 'radios'
    );

    // In case of View options selected, select Views
//     if (module_exists('views')) {
//       $form[$vocab->vid]['taxonomy_menu_show_view_'. $vocab->vid] = array(
//         '#default_value'  => variable_get('taxonomy_menu_show_view_'. $vocab->vid, ''),
//         '#options'        => $views_list,
//         '#title'          => t('Views available'),
//         '#type'           => 'select'
//       );
//     }
  }

  // General options
  $form['taxonomy_menu_display_page'] = array(
    '#default_value'  => variable_get('taxonomy_menu_display_page', 'category'),
    '#description'    => t('How should be the first item on the menu? Example: categories/1/2/3, technology/1/2/3'),
    '#title'          => t('Module page'),
    '#type'           => 'textfield',
  );
  $form['taxonomy_menu_display_num'] = array(
    '#default_value'  => variable_get('taxonomy_menu_display_num', FALSE),
    '#description'    => t('If checked, number of node per term will be displayed in the menu.'),
    '#title'          => t('Display number of nodes per terms'),
    '#type'           => 'checkbox',
  );
  $form['taxonomy_menu_hide_empty'] = array(
    '#default_value'  => variable_get('taxonomy_menu_hide_empty', FALSE),
    '#description'    => t('If checked, only taxonomy terms with members will be shown in the menu.'),
    '#title'          => t('Hide Empty Terms'),
    '#type'           => 'checkbox',
  );
  $form['taxonomy_menu_display_descendants'] = array(
    '#default_value'  => variable_get('taxonomy_menu_display_descendants', TRUE),
    '#description'    => t('If checked, then when a term is selected all nodes belonging to subterms are also displayed.'),
    '#title'          => t('Display descendants'),
    '#type'           => 'checkbox',
  );

  $form['submit'] = array(
    '#type'           => 'submit',
    '#value'          => t('Save configuration')
  );

  return $form;
}

/**
 * Admin area. Configure the module, setting which
 * vocabularies will be converted into menus items
 */
function _taxonomy_menu_admin_submit($form, &$form_state) {
  // Save these options
  variable_set('taxonomy_menu_display_page',        $form_state['values']['taxonomy_menu_display_page']);
  variable_set('taxonomy_menu_display_num',         $form_state['values']['taxonomy_menu_display_num']);
  variable_set('taxonomy_menu_hide_empty',          $form_state['values']['taxonomy_menu_hide_empty']);
  variable_set('taxonomy_menu_display_descendants', $form_state['values']['taxonomy_menu_display_descendants']);

  // Save which category should be displayed on menu
  // and save which views it should use, in case Views
  // mode is selected
  foreach (taxonomy_get_vocabularies() as $vocab) {
    variable_set('taxonomy_menu_show_'. $vocab->vid,
      $form_state['values']['taxonomy_menu_show_'. $vocab->vid]);
    variable_set('taxonomy_menu_show_views_'. $vocab->vid,
      $form_state['values']['taxonomy_menu_show_view_'. $vocab->vid]);
  }

  // Rebuild the menu to include these features
  menu_rebuild();
}

/**
 * Implementation of hook_menu().
 *
 * Its the main function for this module.
 */
function _taxonomy_menu_menu() {
  $items['admin/settings/taxonomy_menu'] = array(
    'access arguments'    => array('administer site configuration'),
    'description'         => t('Global configuration of taxonomy menu functionality.'),
    'file'                => 'taxonomy_menu.inc',
    'page callback'       => 'drupal_get_form',
    'page arguments'      => array('_taxonomy_menu_admin'),
    'path'                => 'admin/settings/taxonomy_menu',
    'title'               => t('Taxonomy Menu settings'),
    'type'                => MENU_NORMAL_ITEM
  );

  foreach (taxonomy_get_vocabularies() as $vocab) {
    if (variable_get('taxonomy_menu_show_'. $vocab->vid, TAXONOMY_MENU_NONE)) {
      $path =  variable_get('taxonomy_menu_display_page', 'category') .'/'. $vocab->vid;
      $items[$path] = array(
        'access arguments'    => array('access content'),
        'file'            => 'taxonomy_menu.inc',
        'page callback'   => '_taxonomy_menu_page',
        'title'           => t($vocab->name),
        'weight'          => $vocab->weight
      );

      $tree = taxonomy_get_tree($vocab->vid);
      $old_depth = -1;
      $old_path = $path;

      foreach ($tree as $term) {
        if ($term->depth <= $old_depth) {
          $slashes_to_remove = $old_depth - $term->depth + 1;
          for ($i = 0; $i < $slashes_to_remove; $i++) {
            $old_path = substr($old_path, 0, strrpos($old_path, '/'));
          }
        }
        $path       = $old_path .'/'. $term->tid;
        $old_depth  = $term->depth;
        $old_path   = $path;

        // Calculate the numbers of children nodes
        $num = taxonomy_term_count_nodes($term->tid);

        // If the number of children nodes of this term is
        // zero and the Hide Empty Terms option is enabled,
        // dont create the menu item
        if (variable_get('taxonomy_menu_hide_empty', FALSE) == FALSE or $num != 0) {
          $name = t($term->name);
          if (variable_get('taxonomy_menu_display_num', FALSE) == TRUE) {
            $name .= ' ('. $num .')';
          }
          $items[$path] = array(
            'access arguments' => array('access content'),
            'file'            => 'taxonomy_menu.inc',
            'page callback'   => '_taxonomy_menu_page',
            'description'     => t($term->description),
            'title'           => $name,
            'weight'          => $term->weight,
          );
        }
      }
    }
  }

  return $items;
}

/**
 * Generates the breadcumb for nodes that
 * have a category listed as a menu
 *
 * @param
 *   Object. The node object
 * @param
 *   Array. The list of all taxonomy vocabs and
 *   terms that this node have and are also
 *   menus
 */
function _taxonomy_menu_node_view(&$node, &$vocabs) {
  foreach ($vocabs as $vid => $vocab) {
    $path = variable_get('taxonomy_menu_display_page', 'category') .'/'. $vid;

    $tree = taxonomy_get_tree($vid);
    $old_depth = -1;
    $old_path = $path;

    // Generate the entire breadcumb
    foreach ($tree as $term) {
      if ($term->depth <= $old_depth) {
        $slashes_to_remove = $old_depth - $term->depth + 1;
        for ($i = 0; $i < $slashes_to_remove; $i++) {
          $old_path = substr($old_path, 0, strrpos($old_path, "/"));
        }
      }
      $path       = $old_path .'/'. $term->tid;
      $old_depth  = $term->depth;
      $old_path   = $path;

      // When we reach the term that this node uses,
      // set the breadcumb
      if ($term->tid == $vocab[0]) {
        drupal_set_breadcrumb(array(
          array('path' => $path, 'title' => t($term->name)),
          array('path' => 'node/'. $node->nid, 'title' => $node->title)
        ));

        // Quit after the first match.
        return;
      }
    }
  }
}

/**
 * Page callback that renders a node listing for the selected term.
 */
function _taxonomy_menu_page() {
  // Check if the Vocabulary ID is set
  if ($vid = arg(1)) {

    // Depending on what Output technique is used,
    // show the nodes' list
    if (variable_get('taxonomy_menu_show_'. $vid, TAXONOMY_MENU_NONE) == TAXONOMY_MENU_NORMAL) {
      if ($tid = arg(2)) {
        $tid = explode('/', $_GET['q']);
        $tid = db_escape_string(array_pop($tid));

        // Add the RSS feed
        $feed = url('taxonomy/term/'. $tid .'/'.
          (variable_get('taxonomy_menu_display_descendants', 1) ? 'all' : 0) .
          '/feed');
        drupal_add_feed($feed);

        // Get the nodes
        $result = taxonomy_select_nodes(array($tid), 'or',
          variable_get('taxonomy_menu_display_descendants', 1) ? 'all' : 0);
      }
      else {
        // If no arg(2), we're looking at just the vid. If
        // display_descendants is on, grab all terms regardless
        // of depth. If off, grab depth 0 terms.
        $tree = taxonomy_get_tree($vid);
        $descendants = variable_get('taxonomy_menu_display_descendants', 1);
        foreach ($tree as $term) {
          if ($descendants or $term->depth == 0) {
            $tids[] = $term->tid;
          }
        }

        // The requested terms have already been determined,
        // so don't request descendants here.
        $result = taxonomy_select_nodes($tids, 'or', 0);
      }

      // Render the selected nodes
      $output = taxonomy_render_nodes($result);
    }
    elseif (variable_get('taxonomy_menu_show_'. $vid, TAXONOMY_MENU_NONE) == TAXONOMY_MENU_VIEW) {
      // Get the last page argument
      $tid = explode('/', $_GET['q']);
      $tid = db_escape_string(array_pop($tid));

      $arguments[] = $vid;

      // Only add the Term ID if its not the Vocabulary ID
      if ($vid != $tid) {
        $arguments[] = $tid;
      }

      // Embed the views output into the page
      $output = views_build_view('embed',
        views_get_view(variable_get('taxonomy_menu_show_views_'. $vid, '')),
        $arguments, FALSE, NULL);
    }
    elseif (variable_get('taxonomy_menu_show_'. $vid, TAXONOMY_MENU_NONE) == TAXONOMY_MENU_DEFAULT_TAX) {

    }
  }

  // If no content found, return a "error" message
  return empty($output) ? t('No content for this category.') : $output;
}
