<div class="clearfix">
  <h2>Account</h2>
  <input style="display:none;" type="password" name="noautofill" />
  <?php print render($form['account']['name']); ?>
  <?php print render($form['account']['current_pass']); ?>
  <?php print render($form['account']['pass']); ?>
  <?php print render($form['account']['mail']); ?>
  <?php print render($form['field_newsletter']); ?>
  <div class="grey smaller">Did you know? You get a 50% permanent credit boost while being subscribed to the newsletter!</div>
</div>

<div class="form-actions">
  <?php print render($form['actions']['submit']); ?>
</div>

<?php print render($form['form_id']); ?>
<?php print render($form['form_token']); ?>
<?php print render($form['form_build_id']); ?>
