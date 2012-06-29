<?php

/**
 * Provides and CRUD functions needed for adding or removing users from your openscholar site
 *
 * @file user.inc
 *
 * Provides user management and crud functions
 *
 * os_user_create
 *   Creates a new drupal user and (optionally) automatically adds them to the current site
 *
 * os_user_add
 *  Add users to a OpenScholar site (multi-tenant)
 *
 * os_user_remove
 *  Removes a user from an Openscholar site (multi-tenant)
 *
 * os_role_grant
 *   Grant a user a role (standard or OG depending on multi-tenant)
 *
 * os_role_revoke
 *   Revoke a users role (standard or OG depending on multi-tenant)
 *
 */

/**
 * Creates a new drupal user
 *
 * @param $user_info
 *   An array of fields and values to save. For example array('name'
 *   => 'My name'). Key / value pairs added to the $edit['data'] will be
 *   serialized and saved in the {users.data} column.
 *
 * @param $add_user
 * If true and you are in a group add them to your group
 *
 * @return $account
 *   The new user account or false if there was an error
 */
function os_user_create($user_info, $add_user = TRUE){
  
  $account = user_save(null,$user_info);
  
  if(!$account || !isset($account->uid)){
    return false;
  }
  
  if($add_user){
    os_user_add($account->uid);
  }
  
  return $account;
}

/**
 * Add users to a OpenScholar site
 *
 * @param int $uid
 * The identifier string for a context
 *
 * @param $site_id
 * Site ID
 *
 * @param $state
 * User state
 *
 * @param $group_type
 * The group type you would like to add to
 *
 * @return bool
 * Success
 */
/**
 * Add a user, and if you are in a group add them to your group
 */
function os_user_add($uid, $site_id = false, $state = OG_STATE_ACTIVE, $group_type = 'node'){
  
  if(!$site_id){
    if(module_exists('vsite') && ($vsite = spaces_get_space())){
      $site_id = $vsite->id;
    }else{
      //No SID passed and we are not in a site
      return false;
    }
  }
  
  $account = user_load($uid);
  //Do some basic validation
  if (!$account) {
    //An invalid user has been passed
    return false;
  }
  elseif (og_get_membership($group_type, $site_id, 'user', $uid)) {
    //The User is already a member of this group
    return false;
  }
  // Check field cardinality.
  elseif (!og_check_field_cardinality('user', $account, 'og_user_group_ref')) {
    //This group has reached the maximum number of members
    return false;
  }
  
  $values = array();
  $og_membership = og_membership_create($group_type, $site_id, 'user', $uid, 'og_user_group_ref', $values);
  
  // Ungroup user, in case they were already registered.
  og_ungroup($group_type, $site_id, 'user', $account);

  $og_membership->etid = $uid;
  $og_membership->state = $state;
  $og_membership->save();
  
  return true;
}

/**
 * Grant a role to a user.
 *
 * @param $uid
 *   The user ID.
 * @param $rid
 *   The role ID.
 */
function os_role_grant($uid, $role_name, $site_id = false, $group_type = 'node') {
  
  if(!$site_id && module_exists('vsite')){
    $vsite = spaces_get_space();
    if($vsite){
      $site_id = $vsite->id;
    }else{
      //not in a site
      return false;
    }
  }
  
  if($site_id){
    //Add role via OG
    $roles = og_roles($group_type, $site_id, 'vsite');
    foreach ($roles as $rid => $role) {
    	if($role == $role_name){
    	  og_role_grant($group_type, $site_id, $uid, $rid);
    	  return true;
    	}
    }
  }else{
    //Add role via standard drupal
    $role = user_role_load_by_name($role_name);
    if($role){
      $user = user_load($uid,TRUE);
      $user->role[$role->rid] = $role;
      user_save($user);
      return true;
    }
  }
  
  return false;
}

/**
 * Revoke a role from a user.
 *
 * @param $uid
 *   The user ID.
 * @param $role_name
 *   The role name.
 */
function os_role_revoke($uid, $role_name, $site_id = false, $group_type = 'node') {
  
  if(!$site_id && module_exists('vsite')){
    $vsite = spaces_get_space();
    if($vsite){
      $site_id = $vsite->id;
    }else{
      //not in a site
      return false;
    }
  }
  
  if($site_id){
    //Remove role via OG
    $roles = og_roles($group_type, $site_id, 'vsite');
    foreach ($roles as $rid => $role) {
      if($role == $role_name){
        og_role_revoke($group_type,$site_id,$uid,$rid);
        return true;
      }
    }
  }else{
    //Add role via standard drupal
    $role = user_role_load_by_name($role_name);
    if($role){
      $user = user_load($uid,TRUE);
      if(isset($user->role[$role->rid])){
        unset($user->role[$role->rid]);
        user_save($user);
      }
      return true;
    }
  }
  
  return false;
  
  
  
}

/**
 * Remove users from a OpenScholar site
 *
 * @param int $uid
 * The identifier string for a context
 *
 * @param $site_id
 * Site ID
 *
 * @param $group_type
 * The group type you would like to add to
 *
 * @return bool
 * Success
 */
/**
 * Add a user, and if you are in a group add them to your group
 */
function os_user_remove($uid, $site_id = false, $group_type = 'node'){
  $account = user_load($uid);
  
  //Do some basic validation
  if (!$account) {
    //An invalid user has been passed
    return false;
  }
  
  if(!$site_id){
    if(module_exists('vsite') && ($vsite = spaces_get_space())){
      $site_id = $vsite->id;
    }else{
      //No SID passed and we are not in a site
      return false;
    }
  }
  
  og_ungroup($group_type, $site_id, 'user', $uid);
  
  return true;
}