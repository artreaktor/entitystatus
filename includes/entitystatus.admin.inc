<?php

/**
 * @file
 * Contains administrative pages for creating, editing, and deleting entity statuses.
 */

/**
 * Entity status administration page.
 */
function entitystatus_admin_page() {

  //return 'Entity status conf page';
  $statuses = entitystatus_get_statuses();

  return theme('entitystatus_admin_page', array(
    'statuses' => $statuses,
  ));
}

/**
 *
 */
function theme_entitystatus_admin_page($variables) {
  $out = '';
  $statuses = $variables['statuses'];

  foreach ($statuses as $status) {
    $ops = array(
      'status_edit' => array('title' => t('edit'), 'href' => $status->admin_path['edit']),
      'status_delete' => array('title' => t('delete'), 'href' => $status->admin_path['delete']),
    );

    $row = array();
    $row[] = check_plain($status->name);
    $row[] = $status->entity_type;
    $row[] = $status->bundle_type ? implode(', ', $status->bundle_type) : '-';
    $row[] = theme('links', array('links' => $ops));

    $rows[] = array(
      'data' => $row,
    );
  }
  if (!$statuses) {
    $rows[] = array(
      array('data' => t('No statuses are currently defined.'), 'colspan' => 4),
    );
  }

  $header = array(
    t('Entitystatus Set name'),
    t('Entity type'),
    t('Entity bundles'),
    t('Operations'),
  );

  $out .= theme('table', array(
    'header' => $header,
    'rows' => $rows,
    'attributes' => array('id' => 'entitystatus-admin-listing-table'),
  ));

  return $out;
}

/**
 * Menu callback for adding a new entity status.
 */
function entitystatus_add_page($entity_type = NULL) {
  drupal_set_title(t('Select entity type'));
  return drupal_get_form('entitystatus_entity_form');
}

/**
 * Delete entitystatus page.
 */
function entitystatus_delete_confirm($form, &$form_state, $sid) {
  $ent = entity_load('entitystatus_set', array($sid));
  $status = $ent[$sid];
  $form['#status'] = $status;
  return confirm_form($form,
    t('Are you sure you want to delete %title?', array('%title' => $status->name)),
    !empty($_GET['destination']) ? $_GET['destination'] : 'admin/structure/entitystatus',
    t('Entity status and all related used input will be deleted. This action cannot be undone.'),
    t('Delete'), t('Cancel')
  );
}

function entitystatus_delete_confirm_submit($form, &$form_state) {
  $status = $form['#status'];
  if ($form_state['values']['confirm']) {

    entity_delete('entitystatus_set', $status->sid);
    // Delete all related options
    db_delete('entitystatus_options')
      ->condition('sid', $status->sid)
      ->execute();

  }
  drupal_set_message(t('Entity status @name has been deleted.', array('@name' => $status->name)));
  $form_state['redirect'] = 'admin/structure/entitystatus';
}