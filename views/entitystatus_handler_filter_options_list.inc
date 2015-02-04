<?php

/**
 * Define my custom in_opeator
 */
class entitystatus_views_handler_filter_options_list extends views_handler_filter_in_operator {

  var $validated_exposed_input = NULL;

  function init(&$view, &$options) {
    parent::init($view, $options);
    if (isset($this->definition['entity statuses'])) {
      $tables = array_keys($this->definition['entity statuses']);
      if (in_array($this->view->base_table, $tables)) {
        $this->options['entitystatus'] = $this->definition['entity statuses'][$this->view->base_table];
      }
    }
  }

  function has_extra_options() {
    return TRUE;
  }

  function get_value_options() {
    if (isset($this->value_options)) {
      return;
    }
    $sid = $this->options['entitystatus_set'];
    $this->value_options = $this->options['entitystatus'][$sid]->options;
    return $this->value_options;
  }

  function operators() {
    $operators = parent::operators();
    return $operators;
  }

  function option_definition() {
    $options = parent::option_definition();
    $options['entitystatus'] = array('default' => 0);
    $options['error_message'] = array('default' => TRUE, 'bool' => TRUE);
    return $options;
  }


  function extra_options_form(&$form, &$form_state) {

    $options = array();
    foreach ($this->options['entitystatus'] as $status_set) {
      $options[$status_set->sid] = check_plain($status_set->name);
    }

    $form['entitystatus_set'] = array(
      '#type' => 'radios',
      '#title' => t('Select Entitystatus Options set'),
      '#options' => $options,
      '#default_value' => $this->options['entitystatus_set'],
    );
  }

  function value_form(&$form, &$form_state) {
    $form['value'] = array();
    $options = array();

    if (empty($form_state['exposed'])) {
      // Add a select all option to the value form.
      $options = array('all' => t('Select all'));
    }

    $this->get_value_options();

    $options += $this->value_options;
    $default_value = (array) $this->value;

    $which = 'all';
    if (!empty($form['operator'])) {
      $source = ($form['operator']['#type'] == 'radios') ? 'radio:options[operator]' : 'edit-options-operator';
    }
    if (!empty($form_state['exposed'])) {
      $identifier = $this->options['expose']['identifier'];

      if (empty($this->options['expose']['use_operator']) || empty($this->options['expose']['operator_id'])) {
        // Exposed and locked.
        $which = in_array($this->operator, $this->operator_values(1)) ? 'value' : 'none';
      }
      else {
        $source = 'edit-' . drupal_html_id($this->options['expose']['operator_id']);
      }

      if (!empty($this->options['expose']['reduce'])) {
        $options = $this->reduce_value_options();

        if (!empty($this->options['expose']['multiple']) && empty($this->options['expose']['required'])) {
          $default_value = array();
        }
      }

      if (empty($this->options['expose']['multiple'])) {
        if (empty($this->options['expose']['required']) && (empty($default_value) || !empty($this->options['expose']['reduce']))) {
          $default_value = 'All';
        }
        elseif (empty($default_value)) {
          $keys = array_keys($options);
          $default_value = array_shift($keys);
        }
        else {
          $copy = $default_value;
          $default_value = array_shift($copy);
        }
      }
    }

    if ($which == 'all' || $which == 'value') {
      $form['value'] = array(
        '#type' => $this->value_form_type,
        '#title' => $this->value_title,
        '#options' => $options,
        '#default_value' => $default_value,
        // These are only valid for 'select' type, but do no harm to checkboxes.
        '#multiple' => TRUE,
        '#size' => count($options) > 8 ? 8 : count($options),
      );
      if (!empty($form_state['exposed']) && !isset($form_state['input'][$identifier])) {
        $form_state['input'][$identifier] = $default_value;
      }

      if ($which == 'all') {
        $form_elements = array('checkbox', 'checkboxes', 'radios', 'select');
        if (empty($form_state['exposed']) && (in_array($this->value_form_type, $form_elements))) {
          $form['value']['#prefix'] = '<div id="edit-options-value-wrapper">';
          $form['value']['#suffix'] = '</div>';
        }
        $form['value']['#dependency'] = array($source => $this->operator_values(1));
      }
    }
  }

  function value_validate($form, &$form_state) {}

  function value_submit($form, &$form_state) {}

  function query() {

    $this->ensure_my_table();
    $real_field_name = $this->real_field;

    if (isset($this->view->exposed_data[$real_field_name])) {
      $value = array($this->view->exposed_data[$real_field_name]);
    }
    else {
      $value = $this->value;
    }

    switch ($this->operator) {
      case 'empty':
        $this->query->add_where($this->options['group'], "$this->table_alias.oid", '', 'IS NULL');
        break;

      case 'not empty':
        $this->query->add_where($this->options['group'], "$this->table_alias.oid", '', 'IS NOT NULL');
        break;

      case 'not in':
        if (!empty($value)) {
          $this->query->add_where(
            $this->options['group'],
            db_or()
              // User statuses.
              ->condition(db_and()
                  ->condition("$this->table_alias.oid", $value, $this->operator)
              )
              ->condition(db_and()
                  ->condition("$this->table_alias.oid", '', 'IS NULL')
              )
          );
        }
        break;

      default:
        // Don't filter if nothing selected.
        if (!empty($value)) {
          $operator = $this->operator;
          $condition = db_or()->condition("$this->table_alias.oid", $value, $this->operator);
          $this->query->add_where($this->options['group'], $condition);
        }
    }
  }

  function admin_summary() {
    if ($this->value) {
      $this->value = array_filter($this->value);
      $sid = $this->options['entitystatus_set'];
      foreach ($this->value as $oid) {
        $this->value_options[$oid] = $this->options['entitystatus'][$sid]->options[$oid];
      }
    }
    return parent::admin_summary();
  }

}
