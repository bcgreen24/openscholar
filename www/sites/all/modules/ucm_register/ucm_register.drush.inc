<?php
// $Id: example.drush.inc,v 1.6 2009/12/06 12:53:38 weitzman Exp $

function ucm_register_drush_command() {
  $items = array();

  // the key in the $items array is the name of the command.
  $items['ucm-register-process'] = array(
    // a short description of your command
    'description' => "Process registrations for ssh support",
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
  );

  return $items;
}


function ucm_register_drush_help($section) {
  switch ($section) {
    case 'drush:ucm-register-process':
      return dt("Run's through items that need to be processed and adds support where needed.");
  }
}


function drush_ucm_register_process() {
  
  // Define some variables 
//  $autohome_backup = '/local/users/drupadm/autohome_backup/auto_home';
//  $sites_directory = '/cms/graduate_sites';
//  $autohome_location = '/etc/auto_home';
//  $apache_aliases = '/local/users/drupadm/apache_user_conf/aliases';
//  $apache_restrictions = '/local/users/drupadm/apache_user_conf/restrictions';
//  $ucm_register_dir = '/var/opt/webstack/apache2/2.2/htdocs/graduatestudent/gs_ucm_modules/ucm_register';
  $autohome_backup = '/var/www/html/auto_home_backup';
  $sites_directory = '/var/www/html/faculty_sites';
  $autohome_location = '/var/www/html/auto_home';
  $apache_aliases = '/local/users/drupadm/apache_user_conf/aliases';
  $apache_restrictions = '/var/www/html/httpd_conf/restrictions';
  $ucm_register_dir = '/var/www/html/openscholar/sites/all/modules/ucmerced/ucm_register';
  // Get the list of sites that need processing
  $items = ucm_register_items_need_process();
  
  // If we hav some sites process them.
  
  if (count($items) > 0) {
    //backup auto_home
    $timestamp = date('YmdHi');
    drush_print('copy autohome');
    copy($autohome_location, $autohome_backup . '.' . $timestamp);
    if (file_exists($autohome_backup)) {
      unlink($autohome_backup);
    }
 
    copy($autohome_location, $autohome_backup);
  
    // For each site 
    foreach ($items as $item) {
      
      $user_dir = $sites_directory . '/' . $item['name'];
      $user_alias = $apache_aliases . '/' . $item['name'];
      $user_restriction = $apache_restrictions . '/' . $item['name'];
      
      drush_print('remove alias file');
      // remove alias file if it exists.
      if (file_exists($user_alias)) {
        unlink($user_alias);
      }
      
      drush_print('remove restriction');
      // remove restriction file if it exists.
      if (file_exists($user_restriction)) {
        unlink($user_restriction);
      }
      
      // Process self hosted sites
      if (!$item['status']) {
        
        drush_print('make user directory');
        // Make user directory if it doesn't exist and make the user the owner.
        if (!is_dir($user_dir)) {
          mkdir($user_dir);
          copy($ucm_register_dir . '/index.html', $user_dir . '/index.html');
          shell_exec('/usr/bin/sudo /bin/chown -R ' . $item['name'] . ':apache ' . $user_dir);
        }
        
        drush_print('write alias file.');
        // Create apache alias if user is using ssh.
        $handle = fopen($user_alias, 'w');
        fwrite($handle, 'alias /' . $item['name'] . ' ' . $user_dir);
        fclose($handle);
        
        drush_print('write restrictions file');
        // Create apache restriction if user is using ssh.
        $handle = fopen($user_restriction, 'w');
        fwrite($handle, '<Directory "' . $user_dir . '">');
        fwrite($handle, 'php_admin_value open_basedir "' . $user_dir . '"');
        fwrite($handle, '</Directory>');
        fclose($handle);
      }      
      
      drush_print('read autohome file');
      // Process auto_home
      $auto_home_array = file($autohome_backup, FILE_IGNORE_NEW_LINES);
      $auto_home_user_line = $item['name'] . ' -rw,nosuid localhost:' . $user_dir;
      if ($item['status']) {
        // Remove from auto_home
        if ($key = array_search($auto_home_user_line, $auto_home_array)) {
          unset($auto_home_array[$key]);
        }
      }
      else {
        // Add to /etc/auto_home
        if (!array_search($auto_home_user_line, $auto_home_array)) {
          $key = array_search('+auto_home', $auto_home_array);
          $auto_home_array[$key - 1] = $auto_home_user_line;
          $auto_home_array[$key] = '';
          $auto_home_array[$key + 1] = '+auto_home';
        }
      }
      
      $auto_home_output = implode("\n", $auto_home_array);
      
      drush_print('Write autohome file.');
      do {
        if (!($f = fopen($autohome_backup, "wa+"))) {
          break;
        }
        if (!fwrite($f, $auto_home_output)) {
          break;
        }
      } while (0);
      
      if ($f) {
        fclose($f);
      }
      
      // Write the group file
      if ($item['status']) {
        // Remove user from ucmgradstudents group
        drush_print('Remove user from ucmgradstudents group');
        shell_exec('/var/www/html/openscholar/del2ucmgradstudents.sh ' . $item['name']);
      }
      else {
        // Add user to ucmgradstudents group
        drush_print('Add user to ucmgradstudents group');
        shell_exec('/var/www/html/openscholar/add2ucmgradstudents.sh ' . $item['name']);
      }
      
      // Mark the site as processed
      ucm_register_mark_processed($item['name']);
    }
  
    // Copy auto_home back
    drush_print('copy autohome');
    shell_exec('/usr/bin/sudo cp ' . $autohome_backup . ' ' . $autohome_location);
    
    drush_print('refresh apache');
    shell_exec('/usr/bin/sudo /usr/sbin/apachectl restart');
    //shell_exec('/usr/bin/sudo /usr/sbin/svcadm enable sun-apache22');
  }
}
