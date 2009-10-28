<?php
// $Id$
?>
<?php foreach ($contexts as $name => $info): ?>
  <div class="services-context">
    <h2><?php print $info['title'] ?></h2>
    <dl>
      <dt><?php print t('Path') ?></dt>
      <dd><?php print $info['path'] ?></dd>
      <dt><?php print t('Server') ?></dt>
      <dd><?php print $info['server'] ?></dd>
      <dt><?php print t('Authentication') ?></dt>
      <dd><?php print $info['authentication'] ?></dd>
      <dt><?php print t('Server') ?></dt>
      <dd><?php print $info['server'] ?></dd>
      <dt><?php print t('Services') ?></dt>
      <dd>
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
      </dd>
    </dl>
    <?php
      print l(t('Edit !title', array('!title' => $info['title'],)), 
        'admin/build/services/contexts/' . $name,
        array('attributes' => array('class' => 'edit-context'))
      );
    ?>
  </div>
<?php endforeach ?>