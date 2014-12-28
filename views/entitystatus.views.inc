<?php

/**
 * Implements hook_views_data().
 */
function entitystatus_views_data() {
  $data = array();

  $data['entitystatus']['table']['group'] = t('Entity status');
  $data['entitystatus_set']['table']['group'] = t('Entity status');
  $data['entitystatus_options']['table']['group'] = t('Entity status');

  $data['entitystatus']['table']['join'] = array(
    'entitystatus_set' => array(
      'left_field' => 'sid',
      'field' => 'sid',
    ),
    'entitystatus_options' => array(
      'left_field' => 'oid',
      'field' => 'oid',
    ),
  );


  $data['entitystatus']['sid'] = array(
    'title' => t('Entitystatus Set'),
    'help' => t('Entitystatus Set that attached to particular entity.'),
    'relationship' => array(
      'base' => 'entitystatus_set',
      'base field' => 'sid',
      'handler' => 'views_handler_relationship',
      'label' => t('Entitystatus Set relationship'),
      'title' => t('Entitystatus Set'),
      'help' => t('Entitystatus Set that attached to particular entity'),
    ),
  );

  $data['entitystatus']['uid'] = array(
    'title' => t('Entitystatus User uid'),
    'help' => t('The user that set status on item. If you need more user fields add the "Entity status: User" relationship.'),
    'relationship' => array(
      'base' => 'users',
      //'base field' => 'uid',
      'title' => t('Entitystatus User'),
      'help' => t('User who set a status on the entity.'),
      'handler' => 'views_handler_relationship',
      'label' => t('Entitystatus User'),
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_user_name',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_numeric',
    ),
    'field' => array(
      'handler' => 'views_handler_field_user',
    ),
  );

  $data['entitystatus']['oid'] = array(
    'title' => t('Entitystatus Option ID'),
    'help' => t('Unique Option ID'),
    'relationship' => array(
      'base' => 'entitystatus_options',
      'base field' => 'oid',
      'title' => t('Entitystatus Option'),
      'help' => t('Unique Entitystatus Option number.'),
      'handler' => 'views_handler_relationship',
      'label' => t('Entitystatus Option'),
    ),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  $data['entitystatus']['timestamp'] = array(
    'title' => t('Entitystatus time'),
    'help' => t('Content status creation time.'),
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_date',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_date',
    ),
  );

  $data['entitystatus_set']['name'] = array(
    'title' => t('Entitystatus Set name'),
    'help' => t('Entitystatus Set that attached to particular entity.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );

  $data['entitystatus_options']['opt_name'] = array(
    'title' => t('Entitystatus Option name'),
    'help' => t('The name of the option.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'entitystatus_views_handler_filter_options_list',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );

  return $data;
}

/**
 * Implements hook_views_data_alter().
 */
function entitystatus_views_data_alter(&$data) {

  $statuses = entitystatus_get_statuses();
  // Loop through defined statuses and add views configuration
  foreach ($statuses as $status) {

    if (!isset($status->entity_type_info)) {
      continue;
    }

    $info = $status->entity_type_info;

    if (!empty($info['entity_field'])) {

      if ($info['entity_table'] != 'users') {

        $data[$info['entity_table']][$status->mashine_name] = array(
          'group' => t('Entity status'),
          'title' => $status->name . ' form',
          'help' => 'Set/change entitystatus to entity.',
          'real field' => $info['entity_field'],
          'field' => array(
            'label' => t('Entity status'),
            //'handler' => 'entitystatus_handler_field_status_form',
            'handler' => 'entitystatus_handler_field_status_form_rel',
            'entity type' => $status->entity_type,
            'table' => 'entitystatus',
            'field' => 'entity_id',
            'left_table' => $status->entity_type_info['entity_table'],
            'left_field' => $status->entity_type_info['entity_field'],
          ),
        );

        $data[$info['entity_table']][$status->mashine_name.'_rel'] = array(
          'group' => t('Entity status'),
          'title' => $status->name . ' relation',
          'help' => 'Add Entitystatus relationship.',
          'real field' => $info['entity_field'],
          'relationship' => array(
            'handler' => 'views_handler_relationship',
            'label' => t($status->name),
            'base' => 'entitystatus',
            'base field' => 'entity_id',
            'relationship field' => $info['entity_field'],
            'entity type' => $status->entity_type,
          ),
        );
      }

      // Add a relationship for the user
      $data['users']['entitystatus'] = array(
        'group' => t('Entity status'),
        'title' => t("User's entitystatuses"),
        'help' => t('Relate users to the statuses they set on the entities.'),
        'relationship' => array(
          'base' => 'entitystatus',
          'base field' => 'uid',
          'relationship field' => 'uid',
          'handler' => 'views_handler_relationship',
          'label' => t('User entitystatus'),
        ),
      );

      $options_raw = entitystatus_get_options($status->sid);
      $options = array();

      foreach ($options_raw as $opt) {
        $options[$opt->oid] = $opt->opt_name;
      }
      $base_table = $status->entity_type_info['entity_table'];
      $data['entitystatus_options']['opt_name']['filter']['entity statuses'][$base_table][$status->sid] = $status;
      $data['entitystatus_options']['opt_name']['filter']['entity statuses'][$base_table][$status->sid]->options = $options;

    }
  }
}