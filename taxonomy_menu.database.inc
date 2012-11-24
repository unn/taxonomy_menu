<?php

/**
 * @file
 * Database functions.
 */

/**
 * Inserts a taxonomy term/menu item association in {taxonomy_menu} table.
 *
 * @param $mlid
 *   the menu link's ID.
 * @param $tid
 *   the term's ID.
 * @param $vid
 *   the vocabulary's ID.
 * @param $langcode
 *   (optional) Defaults to LANGUAGE_NONE.
 *   The language code associated to the menu link's ID.
 */
function _taxonomy_menu_insert_menu_item($mlid, $tid, $vid, $langcode = LANGUAGE_NONE) {
  $fields = array(
    'mlid' => $mlid,
    'tid' => $tid,
    'vid' => $vid,
    'language' => $langcode,
  );

  db_insert('taxonomy_menu')
    ->fields($fields)
    ->execute();
}


/**
 * Returns an mlid based on a taxonomy term ID and vocabulary ID.
 *
 * @param $tid
 *   the term's ID.
 * @param $vid
 *   the vocabulary's ID.
 * @param $langcode
 *   (optional) Defaults to LANGUAGE_NONE.
 *   The language code associated to the menu link's ID.
 *
 * @return
 *   the menu link ID associated to the $tid and $vid. If not found, returns
 *   FALSE.
 */
function _taxonomy_menu_get_mlid($tid, $vid, $langcode = LANGUAGE_NONE) {
  $query = db_select('taxonomy_menu', 'tm')
    ->fields('tm', array('mlid'))
    ->condition('tid', $tid)
    ->condition('vid', $vid)
    ->condition('language', $langcode);
  $result = $query->execute()->fetchField();

  return $result;
}

/**
 * Retrieves all menu items for a vocabulary.
 *
 * @param $vid
 *   the vocabulary's ID.
 *
 * @return array
 *   An associative array of menu link id / taxonomy term id associations.
 */
function _taxonomy_menu_get_menu_items($vid) {
  $result = db_select('taxonomy_menu', 'tm')
  ->fields('tm', array('mlid', 'tid'))
  ->condition('vid', $vid)
  ->execute()
  ->fetchAllKeyed();

  return $result;
}

/**
 * Deletes all links associated with this vocab from {taxonomy_menu} table.
 *
 * @param $vid
 *   the vocabulary's id to delete items from.
 */
function _taxonomy_menu_delete_all($vid) {
  db_delete('taxonomy_menu')
    ->condition('vid', $vid)
    ->execute();
}

/**
 * Get an array of the tid's related to the node
 *
 * @param $node
 * @return array of tids
 */
function _taxonomy_menu_get_node_terms($node) {
  // Get the taxonomy fields.
  $tids = array();
  $result = db_query("SELECT field_name FROM {field_config} WHERE type = 'taxonomy_term_reference'");
  foreach ($result as $field) {
    $field_name = $field->field_name;

    if (isset($node->$field_name)) {
      $tid_field = $node->$field_name;
      // Loop through all the languages.

      foreach ($tid_field as $tid_field_languages) {
        // Loop through all the tids

        foreach ($tid_field_languages as $tid) {
          $tids[] = $tid['tid'];
        }
      }
    }
  }
  return $tids;
}

/**
 * Get an array of the tid's from the parent
 *
 * @param $tid
 * @return array of tid
 */
function _taxonomy_menu_get_parents($tid) {
  $output = array();
  $result = taxonomy_get_parents($tid);
  foreach ($result as $key => $item) {
    $output[] = $key;
  }
  return $output;
}

/**
  * Delete all rows from {taxomony_menu} associated with this term ID and
  * vocabulary ID.
  *
  * @param $vid
  *   The vocabulary ID.
  * @param $tid
  *   The term ID.
  */
function _taxonomy_menu_delete_item($vid, $tid) {
  db_delete('taxonomy_menu')
    ->condition('vid', $vid)
    ->condition('tid', $tid)
    ->execute();
}

/**
 * Get all of the tid for a given vocabulary id.
 * Used to build path (tid1+tid2+...)
 *
 * @param $vid
 *   The vocabulary id.
 *
 * @return
 *   A list of terms id for this vocabulary.
 */
function _taxonomy_menu_get_tids($vid) {
  $result = db_select('taxonomy_term_data', 'td')
  ->condition('vid', $vid)
  ->fields('td', array('tid'))
  ->execute();
  return $result->fetchCol();
}

/**
 * @TODO Needs Updating since terms are related via node fields
 *
 * used to get the count without children
 *
 * @param $tid
 */
function _taxonomy_menu_term_count($tid) {
  $result = db_select('taxonomy_index', 'tn');
  $result->condition('tid', $tid);
  $result->join('node', 'n', 'n.nid = tn.nid AND n.status = 1');
  $result->addExpression('COUNT(n.nid)', 'term_count');
  $temp = $result->execute();
  $temp = $temp->fetchObject();
  return $temp->term_count;
}

/**
 * Get tid for a given mlid
 *
 * @param $mlid
 * @return $tid
 */
function _taxonomy_menu_get_tid($mlid) {
  $where = array(
    ':mlid' => $mlid,
  );
  return db_query('SELECT tid FROM {taxonomy_menu} WHERE mlid = :mlid', $where)->fetchField();
}

/**
 * Get vid, tid for a given mlid
 *
 * @param $mlid
 * @return array of vid, tid
 */
function _taxonomy_menu_get_item($mlid) {
  $result = db_select('taxonomy_menu', 'tm')
    ->condition('mlid', $mlid, '=')
    ->fields('tm', array('tid', 'vid'))
    ->execute();
  return $result->fetch();
}

/**
 * Get the vocabulary for a tid
 * @param $tid array of tids
 * @return $vid
 */
function _taxonomy_menu_get_vid_by_tid($tids) {
  if ($tids) {
    $result = db_select('term_data')
      ->condition('tid', $tids, 'IN')
      ->fields('term_data', array('vid'))
      ->distinct()
      ->execute();
    $vids = array();
    return $result->fetchAllAssoc('vid');
  }
}
