<?php

class entitystatus_handler_field_status_form_rel extends views_handler_field {

  function init(&$view, &$options) {
    parent::init($view, $options);

    $sid = db_select('entitystatus_set', 'ess')
      ->fields('ess', array('sid'))
      ->condition('ess.mashine_name', $this->options['id'])
      ->execute()
      ->fetchField();

    $statuses = entitystatus_get_statuses($this->definition['entity type']);
    foreach ($statuses as $status) {
      if ($status->mashine_name == $this->options['id']) {
        $this->options['entitystatus_set'] = $status;
      }
    }
  }


  function query() {

    // $this->table = node; - entity table
    // $this->field = bushstatus; - status mashine name
    // $this->definition['entity type'] = 'node'; - entity type name
    // $this->definition['real field'] = 'nid'; - entity field
    // ????????
    // $this->table_alias = 'entitystatus';
    // $this->field_alias = 'entity_id';
    $this->ensure_my_table();
    $join = new views_join();
    $join->table = 'entitystatus';
    $join->left_table = $this->view->base_table;
    $join->left_field = $this->view->base_field;
    $join->field = 'entity_id';
    $join->type = 'LEFT';

    $this->query->add_relationship($join->table, $join, $join->left_table);
    $this->query->add_field($join->table, 'item_id', 'es_id');
    //$this->query->add_field($join->table, 'sid', 'es_sid');

    global $user;

    // User statuses.
    $this->query->add_where(
      0,
      db_or()
        // User statuses.
        ->condition(db_and()
          ->condition('entitystatus.uid', $user->uid, '=')
          ->condition('entitystatus.entity_type', $this->definition['entity type'], '=')
        )
        // Entities without status.
        ->condition(db_and()
          ->condition('entitystatus.entity_type', '', 'IS NULL')
        )
    );

  }

  function render($values) {

    $value = $this->get_value($values);
    $entity_id_field = $this->definition['real field'];

    $entity = entity_load_single($this->definition['entity type'], $values->$entity_id_field);
    $wrapper = entity_metadata_wrapper($this->definition['entity type'], $values->$entity_id_field);
    $entity->content['#entity_type'] = $this->definition['entity type'];
    $entity->content['#bundle'] = $wrapper->getBundle();
    $entity->content['#entity_id'] = $values->$entity_id_field;

    // Prevents form generating on inappropriate entity types
    if (in_array($entity->content['#bundle'], $this->options['entitystatus_set']->bundle_type)) {
      $form = drupal_get_form('entitystatus_ui_form', $this->options['entitystatus_set'], $entity, '');
      $form_output = drupal_render($form);
      return $form_output;
    }
    else {
      return '';
    }

  }

}
