<div class="clearfix">
  <?php print render($form['account']['name']); ?>
  <?php print render($form['account']['mail']); ?>
  <?php print render($form['terms']); ?>
  <?php print render($form['field_newsletter']); ?>
  <?php print render($form['captcha']); ?>
  <div class="form-actions">
    <?php print render($form['actions']['submit']); ?>
  </div>
</div>
<?php print render($form['form_id']); ?>
<?php print render($form['form_token']); ?>
<?php print render($form['form_build_id']); ?>
