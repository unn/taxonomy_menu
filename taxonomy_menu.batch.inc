<?php

/**
 * @file
 * Batch API Code.
 */

/**
 * Saves link items in a menu based on available taxonomy terms.
 *
 * @param $terms
 *   A list of taxonomy term objects to build the menu items upon.
 * @param $menu_name
 *   The machine name of the menu in which the menu links should be inserted.
 */
function _taxonomy_menu_save_menu_links_batch($terms, $menu_name) {
  $batch = array(
    'operations' => array(
      array('_taxonomy_menu_save_menu_links_process', array($terms, $menu_name)),
    ),
    'finished' => '_taxonomy_menu_save_menu_links_finished',
    'title' => t('Rebuilding Taxonomy menu'),
    'init_message' => t('The menu items have been deleted, and are about to be regenerated.'),
    'progress_message' => t('Import progress: Completed @current of @total operations.'),
    'redirect' => 'admin/structure/taxonomy',
    'error_message' => t('The Taxonomy menu creation process has encountered an error.'),
  );
  batch_set($batch);
}

/**
 * Processes the batch.
 *
 * @param $terms
 *   A list of taxonomy term objects to build the menu items upon.
 * @param $menu_name
 *   The machine name of the menu in which the menu links should be inserted.
 */
function _taxonomy_menu_save_menu_links_process($terms, $menu_name, &$context) {
  // Terms are processed 10 by 10.
  $limit = 10;

  // Initialize the sandbox the first time through.
  if (!isset($context['sandbox']['progress'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['max'] = count($terms);
  }

  // Store all the terms in the sandbox and retrieve the next
  // group of terms to be processed.
  $context['sandbox']['chunks'] = array_chunk($terms, $limit);

  // Loop through eack chunk and process its terms.
  foreach ($context['sandbox']['chunks'] as $chunk) {
    foreach ($chunk as $term) {
      // Save the menu link for this taxonomy term.
      taxonomy_menu_menu_link_save($term, $menu_name);

      // Store some result for post-processing in the finished callback.
      $context['results'][] = check_plain($term->name);
      // Update our progress information.
      $context['sandbox']['progress']++;
      $context['sandbox']['current_term'] = $term->tid;
    }

    // Inform the batch engine that we are not finished, and provide an estimation
    // of the completion level we reached.
    if ($context['sandbox']['progress'] < $context['sandbox']['max']) {
      $current_count = $context['sandbox']['max'] - $context['sandbox']['progress'];
      $context['message'] = t('Remaining %current taxonomy terms to process out of %count ...', array('%current' => $current_count, '%count' => $context['sandbox']['max']));
      $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
    }
    else {
      $context['finished'] = 1;
    }
  }
}

/**
 * Batch 'finished' callback.
 */
function _taxonomy_menu_save_menu_links_finished($success, $results, $operations) {
  if ($success) {
    $message = count($results) . ' taxonomy terms were processed.</br>';
    $message .= t('The Taxonomy menu has been created.');
  }
  else {
    // An error occurred.
    // $operations contains the operations that remained unprocessed.
    $error_operation = reset($operations);
    $message = t('An error occurred while processing %error_operation with arguments: @arguments', array('%error_operation' => $error_operation[0], '@arguments' => print_r($error_operation[1], TRUE)));
  }
  // Clear cache after saving menu links.
  menu_cache_clear_all();
}
