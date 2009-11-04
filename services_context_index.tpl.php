<?php
// $Id$
drupal_add_css(drupal_get_path('module', 'services') . '/css/admin.css');
?>
<?php foreach ($contexts as $name => $info): ?>
  <div class="services-context">
    <h2><?php print $info['title'] ?></h2>
    <div class="context-info first">
      <div class="info">
        <label><?php print t('Path') ?></label>
        <span class="value"><?php print $info['path'] ?></span>
      </div>
      <div class="info">
        <label><?php print t('Server') ?></label>
        <span class="value"><?php print $info['server'] ?></span>
      </div>
      <div class="info">
        <label><?php print t('Authentication') ?></label>
        <span class="value"><?php print $info['authentication'] ?></span>
      </div>
    </div>
    <div class="context-info last">
      <div class="info">
        <label><?php print t('Services') ?></label>
        <span class="value">
          <ul>
            <?php foreach ($info['resources'] as $rname => $conf): ?>
              <li>
                <?php 
                  if (isset($conf['alias'])) {
                    print t('!alias (!name)', array(
                      '!alias' => $conf['alias'],
                      '!name' => $rname,
                    ));
                  }
                  else {
                    print $rname;
                  }
                ?>
              </li>
            <?php endforeach ?>
          </ul>
        </span>
      </div>
    </div>
    <?php
      print l(t('Edit !title', array('!title' => $info['title'],)), 
        'admin/build/services/contexts/' . $name,
        array('attributes' => array('class' => 'edit-context'))
      );
    ?>
  </div>
<?php endforeach ?>