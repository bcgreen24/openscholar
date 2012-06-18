<?php
/**
 * @file
 * Adativetheme implementation to present a Panels layout.
 *
 * Available variables:
 * - $content: An array of content, each item in the array is keyed to one
 *   panel of the layout.
 * - $css_id: unique id if present.
 * - $panel_prefix: prints a wrapper when this template is used in certain context,
 *   such as when rendered by Display Suite or other module - the wrapper is
 *   added by Adaptivetheme in the appropriate process function.
 * - $panel_suffix: closing element for the $prefix.
 *
 * @see adaptivetheme_preprocess_three_col()
 * @see adaptivetheme_preprocess_node()
 * @see adaptivetheme_process_node()
 */
?>
<?php print $panel_prefix; ?>
<div class="at-panel panel-display three-col clearfix" <?php if (!empty($css_id)) { print "id=\"$css_id\""; } ?>>
  <?php if ($content['three_col_top']): ?>
    <div class="region region-three-col-top region-conditional-stack">
      <div class="region-inner clearfix">
        <?php print $content['three_col_top']; ?>
      </div>
    </div>
  <?php endif; ?>
  <div class="region region-three-col-first">
    <div class="region-inner clearfix">
      <?php print $content['three_col_first']; ?>
    </div>
  </div>
  <div class="region region-three-col-second">
    <div class="region-inner clearfix">
      <?php print $content['three_col_second']; ?>
    </div>
  </div>
  <div class="region region-three-col-third">
    <div class="region-inner clearfix">
      <?php print $content['three_col_third']; ?>
    </div>
  </div>
  <?php if ($content['three_col_bottom']): ?>
    <div class="region region-three-col-bottom region-conditional-stack">
      <div class="region-inner clearfix">
        <?php print $content['three_col_bottom']; ?>
      </div>
    </div>
  <?php endif; ?>
</div>
<?php print $panel_suffix; ?>
