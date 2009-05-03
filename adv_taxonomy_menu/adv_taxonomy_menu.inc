<?php
/**
 * $Id$
 * @file adv_taxonomy_menu.inc
 * * @author Jonathan Chaffer   <jchaffer@structureinteractive.com> original taxonomy_menu.module
 * @author Bruno Massa        <http://drupal.org/user/67164> original taxonomy_menu.module
 * @author Kent Parker <kent@webdev.passingphase.co.nz>  adv_taxonomy_menu.module
*   It Generates menu links for all taxonomy terms
 */

/**
 * Admin area. Configure the module, setting which
 * vocabularies will be converted into menus items
 *
 * @return
 *   Array. The form fields.
 */

function theme_adv_taxonomy_menu_admin_item($setting) {
  return '<td>'.$setting->name.'</td><td><a href="'.url('admin/settings/adv_taxonomy_menu').'/edit/'. $setting->tmid .'">edit</a></td><td><a href="'.url('admin/settings/adv_taxonomy_menu').'/delete/'. $setting->tmid .'">delete</a></td>';
}

function _adv_taxonomy_menu_rebuild_form() {
  $form['submit'] = array(
    '#value'          => t('Rebuild Menu'),
    '#type'           => 'submit'
  );
  return $form;
}
function _adv_taxonomy_menu_rebuild_form_submit() {
  menu_rebuild();
  drupal_set_message('The menu has been rebuilt');
}
function theme_adv_taxonomy_menu_admin($array) {
  if(!empty($array)) {
    drupal_add_css(drupal_get_path('module', 'adv_taxonomy_menu') .'/adv_taxonomy_menu.css');
    $output = t('Current available menu systems');
    $output .= '<table id="taxonomy_menu">';
    foreach($array as $item) $output .= '<tr>'.$item.'</tr>';
    $output .= '</table>';
    $output .= drupal_get_form('_adv_taxonomy_menu_rebuild_form');
  }
  else $output = t('There are currently no taxonomy menu systems created');
  return $output;
}

function _adv_taxonomy_menu_admin($op) {
 if($op == 'main') {
  $output = '';
  if($settings = adv_taxonomy_menu_get_settings()) {
    while ($setting = db_fetch_object($settings)) {
 	  $output[] = theme('adv_taxonomy_menu_admin_item', $setting);
    }
  }
  return theme('adv_taxonomy_menu_admin', $output);
 } else {
  if($op=="delete") {
    if(arg(4)) {
	  $tmid = arg(4);
 	 return adv_taxonomy_menu_delete_form($tmid);
    }
    else $error = "yes";
  }
  if($op=="edit") {
    if(arg(4)) {
	  $tmid = arg(4);
      if($result = adv_taxonomy_menu_get_settings($tmid)) {
        $settings = db_fetch_object($result);
        $order = unserialize($settings->vocab_order);
      }
      $form['adv_taxonomy_menu_tmid'] = array(
          '#type' => 'hidden',
          '#value' => $tmid,
        );
    }
    else $error = "yes";
  }
  $options = array(
    ADV_TAXONOMY_MENU_NORMAL  => t('Normal')
  );

  // If the Views module is enabled, add some special
  // new features
    $level = array("not selected");
    $i = 1;
	$num_levels = 0;
    foreach(taxonomy_get_vocabularies() as $vocab) {
	  $level[] = $i;
	  if($order[$vocab->vid] >0 ) $num_levels++;
      $i++; 
	}
 
    $form['adv_taxonomy_menu_return'] = array(
      '#value'          => '<a href="'.url('admin/settings/adv_taxonomy_menu').'">Return to main administration page</a>',
      '#type'           => 'markup',
    ); 
  // Create some options for each of the vocabularies
   $form['adv_taxonomy_menu_name'] = array(
    '#default_value'  => $settings->name,
    '#description'    => t('Choose a name to identify the menu system by'),
    '#title'          => t('Name of menu system'),
    '#type'           => 'textfield',
  );
  foreach (taxonomy_get_vocabularies() as $vocab) {
    $form[$vocab->vid] = array(
      '#title'          => $vocab->name,
      '#tree'           => FALSE,
      '#type'           => 'fieldset',
    );
    $form[$vocab->vid]['adv_taxonomy_menu_vocab_order_'. $vocab->vid] = array(
      '#title' => t('Menu Level'),
      '#type' => 'select',
      '#options'        => $level,
      '#default_value'  => $order[$vocab->vid],
      '#description' => t('Enter a number to select the level of this vocabulary in the menu.  If you do not select this vocabulary in the menu then this is ignored.'),
      '#required' => FALSE,
    );

  }

    $form[$vocab->vid]['adv_taxonomy_menu_show'] = array(
      '#default_value'  => $settings->show_normal,
      '#options'        => $options,
      '#title'          => t('Default or custom output.  If you select view then you will need to construct a full sql to select nodes according to terms IDs shown in the url (This feature currently not available). Default applies'),
      '#type'           => 'radios',
    );
    // In case of View options selected, select Views - Currently not active
    if (module_exists('viewsxx')) {
      // Add the Views file with more functions
      require_once(drupal_get_path('module', 'views') .'/views_cache.inc');

      // Add a new options on Categories
      $options[ADV_TAXONOMY_MENU_VIEW] = t('Views');

      // Get the list of User generated views
      $views = db_query("SELECT * FROM {view_view}");
      while ($view = db_fetch_array($views)) {
	    //drupal_set_message(print_r($view, 1));
        $views_list[$view['vid']] = $view['name'];
      }

      // Now get a list of default Views
      foreach (_views_get_default_views() as $view => $viewdata) {
        $views_list[$viewdata->vid] = $viewdata->name;
      }
      $form[$vocab->vid]['adv_taxonomy_menu_show_view'] = array(
        '#default_value'  => $settings->show_views,
        '#options'        => $views_list,
        '#title'          => t('Views available'),
        '#type'           => 'select',
      );
    }
  // General options
  $form['adv_taxonomy_menu_num_levels'] = array(
    '#value'    =>       t('The number of levels of this menu is currently set at '.$num_levels),
    '#type'           => 'markup',
  );
  $form['adv_taxonomy_menu_display_page'] = array(
    '#default_value'  => $settings->display_page,
    '#description'    => t('What is the label to use in the url? Example: categories/1/2/3, technology/1/2/3'),
    '#title'          => t('Module page'),
    '#type'           => 'textfield',
  );
  $form['adv_taxonomy_menu_display_num'] = array(
    '#default_value'  => $settings->display_num,
    '#description'    => t('If checked, number of node per term will be displayed in the menu.'),
    '#title'          => t('Display number of nodes per terms'),
    '#type'           => 'checkbox',
  );
  $form['adv_taxonomy_menu_hide_empty'] = array(
    '#default_value'  => $settings->hide_empty,
    '#description'    => t('If checked, only taxonomy terms with members will be shown in the menu.'),
    '#title'          => t('Hide Empty Terms'),
    '#type'           => 'checkbox',
  );
  $form['adv_taxonomy_menu_display_descendants'] = array(
    '#default_value'  => $settings->display_descendants,
    '#description'    => t('If checked, then when a term is selected all nodes belonging to subterms are also displayed.'),
    '#title'          => t('Display descendants'),
    '#type'           => 'checkbox',
  );

  $form['system_mid'] = array(
    '#value'    	  =>  $settings->mid,
    '#type'           => 'hidden',
  );
  $form['submit'] = array(
    '#value'          => t('Submit'),
    '#type'           => 'submit'
  );

  return $form;
}
}

function adv_taxonomy_menu_delete_form($tmid) {
  $menu->nid = $tmid;
  $form['nid'] = array('#type' => 'value', '#value' => $tmid);
  $form['form_id'] = array('#type' => 'value', '#value' => 'delete_menu');
  return confirm_form($form,
   t('Are you sure you want to delete this menu system'),
   isset($_GET['destination']) ? $_GET['destination'] : 'admin/settings/adv_taxonomy_menu',
   t('This action cannot be undone.'),
   t('Delete'), t('Cancel'));
}

function adv_taxonomy_menu_delete_form_submit($form_id, $form_values) {
   db_query("DELETE FROM {adv_taxonomy_menu} WHERE tmid=%d", $form_values['nid']);
   drupal_goto('admin/settings/adv_taxonomy_menu');
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
function _adv_taxonomy_menu_node_view(&$node, &$vocabs) {
  foreach ($vocabs as $vid => $vocab) {
    $path = variable_get('adv_taxonomy_menu_display_page', 'category') .'/'. $vid;

    $tree = taxonomy_get_tree($vid);
    $old_depth = -1;
    $old_path = $path;

    // Generate the entire breadcumb
    foreach ($tree as $term) {
 //drupal_set_message('term '.print_r($term,1));
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
        menu_set_location(array(
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
function _adv_taxonomy_menu_page() {
  // Check if the Vocabulary ID is set
  if ($vid = arg(1)) {
      if($result1 = adv_taxonomy_menu_get_settings($tmid)) {
        $settings = db_fetch_object($result1);
	  }
	$view_id = 0;
	if($settings->show_normal>1) $view_id = $settings->show_views;
    // Depending on what Output technique is used,
    // show the nodes' list
    if ($view_id==0) {
      if ($tid = arg(2)) {
        $tid = explode('/', $_GET['q']);
		// first item is category page, second is vocab id.  Need all tids that follow
		$tids = array();
		foreach($tid as $key => $value) {
		  if($key>1) $tids[] = $value;
		}
	    // allow another module to modify the result set.  Note this will only process ONE module and then exit
		foreach (module_implements('adv_taxonomy_menu_sql_alter') AS $module) {
          $function = $module .'_adv_taxonomy_menu_sql_alter';
		  $result = $function($settings, $tids, 'and', variable_get('adv_taxonomy_menu_display_descendants', TRUE) ? 'all' : 0);
		  break;
		}

        if(!isset($result)) $result = taxonomy_select_nodes($tids, 'and', variable_get('adv_taxonomy_menu_display_descendants', TRUE) ? 'all' : 0);
      }
      else {
        // If no arg(2), we're looking at just the vid. If
        // display_descendants is on, grab all terms regardless
        // of depth. If off, grab depth 0 terms.
        $tree = taxonomy_get_tree($vid);
        $descendants = variable_get('adv_taxonomy_menu_display_descendants', 1);
        foreach ($tree as $term) {
          if ($descendants or $term->depth == 0) {
            $tids[] = $term->tid;
          }
        }
	    // allow another module to modify the result set.  Note this will only process ONE module and then exit
		foreach (module_implements('adv_taxonomy_menu_sql_alter') AS $module) {
          $function = $module .'_adv_taxonomy_menu_sql_alter';
		  $result = $function($settings, $tids, 'or', 0);
		  break;
		}
        // The requested terms have already been determined,
        // so don't request descendants here.
        if(!isset($result)) $result = taxonomy_select_nodes($tids, 'or', 0);
      }
      // Render the selected nodes
	  $output = '';
	  if (db_num_rows($result) > 0) {
	    $nodes = array();
		while ($node = db_fetch_object($result)) {
		  $nodes[] = $node;
	    }
        $output .= theme('adv_taxonomy_menu_output', $nodes);
        $output .= theme('pager', NULL, variable_get('default_nodes_main', 10), 0);
	  }
    }
    elseif ($view_id>0) {
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
        views_get_view($view_id),
        $arguments, FALSE, NULL);
    }
  }

  // If no content found, return a "error" message
  return empty($output) ? t('No content for this category.') : $output;
}

function theme_adv_taxonomy_menu_output($nodes) {
  $output = '<ul class="item-list">';
  foreach($nodes as $node) {
    $output .= '<li>'.l($node->title, 'node/'.$node->nid).'</li>';
  
  }
  $output .= '</ul>';
  return $output;
}