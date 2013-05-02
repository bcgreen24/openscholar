<?php

/**
 * Construct the registration form
 */
function ucm_register_signup_form() {
  $form = array();
  
  // clear any drupal_messages. Ajax takes care of them
  $form['#prefix'] = '<div id = "user_register">';
  $form['#suffix'] = '</div>';  
  
   $form['site_type'] = array(
    '#type' => 'radios',
    '#title' => t('Type of Site'),
    '#description' => t('Choose whether you would like to use our site building software or build your own site from scratch.'),
    '#default_value' => 1,
    '#options' => array(
      1 => t('I want to use the tools available through the site builder and have the UC Merced brand'),
      0 => t('I will build my own site on my local machine and just upload it')
    ),
    '#prefix' => '<div id="site-type-item">',
    '#suffix' => '</div>',
  );
   
  $form['site_type_description_js'] = array(
    '#value' => "<script type='text/javascript'>$(document).ready(function() {
      if ($('#edit-site-type-1:checked')) {
        $('#site-type-description-site-builder').show();
        $('#site-type-description-self-hosted').hide();
      }
      else {
        $('#site-type-description-site-builder').hide();
        $('#site-type-description-self-hosted').show();
      }
      
      $('#edit-site-type-1').click(function() {
        $('#site-type-description-site-builder').show();
        $('#site-type-description-self-hosted').hide();
      });
      
      $('#edit-site-type-0').click(function() {
        $('#site-type-description-site-builder').hide();
        $('#site-type-description-self-hosted').show();
      });
    });</script>"
  );
   
  $form['site_type_description_site_builder'] = array(
    '#value' => '<div id="site-type-description-site-builder"><p>' . t('Easiest way to build your web site, using all the great tools in the system.') . '</p></div>',
  );
  
  $form['site_type_description_self_hosted'] = array(
    '#value' => '<div id="site-type-description-self-hosted"><p>' . t('Your site will be blank and you will need to create your own web site and upload the files using an sftp client.') . '</p></div>',
  );
  
  $form['submit'] = array(
    '#type' => 'submit', 
    '#value' => t('Create your site'), 
    '#weight' => 40,    
  );
    
  return $form;
}

/**
 * the submit handler for the registration form
 */
function ucm_register_signup_form_submit($form, &$form_state) {
  //Account to give owership when creating a new site
  global $user;
  $name = $domain = $user->name;
  $mail  = $user->mail;
  $use_openscholar = $form_state['values']['site_type'];
  
  //install_include(array('user'));
  
  if ($use_openscholar) {
    ucm_register_set_manage($domain, 1, 1);
  }
  else {
    ucm_register_set_manage($domain, 0, 1);
  }
  
  
  $rid = array_search(variable_get('scholar_manager_role', 'faculty'), user_roles());
  install_add_user_to_role($user->uid, $rid); //need to add to 'faculty' role           
 
  //add $user object to $form_state  so it can be used by subsequent submit functions
  if(is_object($user)){
    $form_state['new_user'] = $user;
  }

  // 2. Create the site
  
  if($use_openscholar){
    $preset = variable_get('scholar_default_preset', 'scholar');
    $vsite = ucm_register_vsite_create($name, $domain, $preset);

  if (!$vsite){
    drupal_set_message(_ucm_register_vsite_error());
    drupal_goto('site/register');
  }
  else {
    // site created successfully add the form values for additional submit fuctions
    $form_state['vsite'] = $vsite;
    // 3. send e-mail to the user (unless specified not to by user with special access) 
    $send_mail =  (!$form_state ['values'] ['send_mail'] || $form_state ['values'] ['send_mail'] == 1) ? ucm_register_notify_user($vsite, $user) : FALSE;
  }
  
  //if user has create permissions and not admin give user admin rights to og/vsite
  og_save_subscription($vsite->sid, $user->uid, array('is_active' => 1, 'is_admin' => 1));
  
  //call any other submit functions here
  foreach($form['#submit'] as $fnc){
    if (__FUNCTION__ !== $fnc){
      call_user_func($fnc, $form, $form_state);
    }
  }
  
  global $base_url;
  if ($use_openscholar) {
    // let user know this was all sucessful
    drupal_set_message(ucm_register_message($form, $domain));

    drupal_goto($base_url . '/' . $domain);
  }
  else {
      //drupal_set_message("SSH site created.");
    //drupal_goto($base_url . '/' . $domain);
  } 
 }else{
     //Create alias file in /var/www/html/httpd_conf/aliases
     $alias_file = '/var/www/html/httpd_conf/aliases/' . $domain;
     $data = <<<EOF
        Alias /$domain /var/www/html/faculty_sites/$domain
EOF;
     $handle = fopen($alias_file, 'w');
     fwrite($handle, $data);
     fclose($handle);
     //create home directory in /var/www/html/faculty_sites
     $result = mkdir('/var/www/html/faculty_sites/' . $domain);
     if($result){
         $data = <<<EOF
           <html>
               <head>
                 <title>Faculty Site of $domain</title>
               </head>
                <body>
                Welcome to your new site.
                </body>
          </html>
EOF;
         $handle = fopen('/var/www/html/faculty_sites/'.$domain.'/index.html', 'w');
         fwrite($handle, $data);
         fclose($handle);
         drupal_set_message("Created home folder");
     }else{
         drupal_set_message("Failed to create home folder.");
     }
     //restart apache
     exec('sudo apachectl restart');
    drupal_goto($base_url . $domain);
 }
}

/**
 * Construct the registration form
 */
function ucm_register_site_switch_form() {
  global $user;
  $form = array();
  
  // clear any drupal_messages. Ajax takes care of them
  $form['#prefix'] = '<div id = "user_register">';
  $form['#suffix'] = '</div>';  
  
   $form['site_type'] = array(
    '#type' => 'radios',
    '#title' => t('Type of Site'),
    '#description' => t('Choose if you want to use our site building software or build your own site from scratch. If you choose to create your own from scratch it may take a minute for the site to become available.'),
    '#default_value' => ucm_register_get_status($user->name),
    '#options' => array(1 => t('I want to use the site builder'), 0 => t("I'll create my own site from scratch")),
    '#prefix' => '<div id="site-type-item">',
    '#suffix' => '</div>',
  ); 
  
  $form['submit'] = array(
    '#type' => 'submit', 
    '#value' => t('Make The Switch'), 
    '#weight' => 40,    
  );
    
  return $form;
}

/**
 * the submit handler for the switching site form
 */
function ucm_register_site_switch_form_submit($form, &$form_state) {
  global $user;
  $domain = $user->name;
  $use_openscholar = $form_state['values']['site_type'];
  
  $message  = '<div id="success-message">';
  $message .= '<p>You have successfully switched your site. It may take up to 2 minutes for your site to be switched over.</p>';
  $message .= '<p>' . l('Click here to go to your site.', $user->name) . '</p>';
  if ($use_openscholar) {
    ucm_register_set_manage($domain, 1, 1);
  }
  else {
    ucm_register_set_manage($domain, 0, 1);
    $message .= '<p>' . l('Click here for instructions on how to upload your site files.', 'about/ftp') . '</p>';
  }
  
  $message .= '</div>';
  
  drupal_set_message($message);
}

// Set its management status
function ucm_register_set_manage($name, $status, $process) {
  if (db_result(db_query("SELECT `name` FROM {vsite_managed} WHERE `name` = '%s'", $name)) !== FALSE) {
    db_query("UPDATE {vsite_managed} SET `status` = %d, `process` = %d WHERE `name` = '%s'", $status, $process, $name);
  }
  // otherwise insert a new record
  else {
    db_query("INSERT INTO {vsite_managed} (`name`, `status`, `process`) VALUES ('%s', %d, %d)", $name, $status, $process);
  }
}

/**
 * Create a scholar vsite
 */
function ucm_register_vsite_create($name, $mail, $domain){
  vsite_include('vsiteapi');
  return vsite_vsite_create($name, $mail, $domain);
}

function _ucm_register_vsite_error(){
 return 'The web site cannot be created. Please fix the errors and try again.';
}
