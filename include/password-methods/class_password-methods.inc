<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2018  FusionDirectory

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

/*
 * \file class_pasword-methods.inc
 * Source code for class password-methods
 */

/*!
 * \brief This class contains all the basic function for password methods
 */
class passwordMethod
{
  var $attrs    = array();
  var $display  = FALSE;
  var $hash     = '';

  protected $lockable = TRUE;

  /*!
   * \brief Password method contructor
   *
   * \param string $dn The DN
   * \param object $userTab The user main tab object
   */
  function __construct($dn = '', $userTab = NULL)
  {
  }

  /*!
   * \brief Get the Hash name
   */
  static function get_hash_name()
  {
    trigger_error("get_hash_name can't be called on main class");
  }

  /*!
   * \brief If we need password
   *
   * \return boolean TRUE
   */
  function need_password()
  {
    return TRUE;
  }

  /*!
   * \brief If we can lock the password
   *
   * \return boolean
   */
  function is_lockable()
  {
    return $this->lockable;
  }

  /*!
   * \brief Is locked
   *
   * \param string $dn The DN
   */
  function is_locked($dn = "")
  {
    global $config;
    if (!$this->lockable) {
      return FALSE;
    }

    /* Get current password hash */
    $pwd = "";
    if (!empty($dn)) {
      $ldap = $config->get_ldap_link();
      $ldap->cd($config->current['BASE']);
      $ldap->cat($dn);
      $attrs = $ldap->fetch();
      if (isset($attrs['userPassword'][0])) {
        $pwd = $attrs['userPassword'][0];
      }
    } elseif (isset($this->attrs['userPassword'][0])) {
      $pwd = $this->attrs['userPassword'][0];
    }
    return preg_match("/^[^\}]*+\}!/", $pwd);
  }

  /*! \brief       Locks an account by adding a '!' as prefix to the password hashes.
   *               This makes login impossible, due to the fact that the hash becomes invalid.
   *               userPassword: {SHA}!q02NKl9IChNwZEAJxzRdmB6E
   *               sambaLMPassword: !EBD223B61F8C259AD3B435B51404EE
   *               sambaNTPassword: !98BB35737013AAF181D0FE9FDA09E
   *
   * \param string $dn
   */
  function lock_account($dn = "")
  {
    return $this->generic_modify_account($dn, 'LOCK');
  }

  /*!
   * \brief Unlocks an account which was locked by 'lock_account()'.
   *        For details about the locking mechanism see 'lock_account()'.
   */
  function unlock_account($dn = "")
  {
    return $this->generic_modify_account($dn, 'UNLOCK');
  }

  /*!
   * \brief Unlocks an account which was locked by 'lock_account()'.
   *        For details about the locking mechanism see 'lock_account()'.
   */
  private function generic_modify_account($dn, $mode)
  {
    global $config;
    if (!$this->lockable) {
      return FALSE;
    }
    if ($mode != 'LOCK' && $mode != 'UNLOCK') {
      throw new FusionDirectoryException('Invalid mode "'.$mode.'"');
    }

    /* Get current password hash */
    $attrs  = $this->attrs;
    $pwd    = '';
    $ldap   = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    if (!empty($dn)) {
      $ldap->cat($dn);
      $attrs = $ldap->fetch();
    }
    if (isset($attrs['userPassword'][0])) {
      $pwd = $attrs['userPassword'][0];
      $dn  = $attrs['dn'];
    }

    /* We can only lock/unlock non-empty passwords */
    if (!empty($pwd)) {
      /* Check if this entry is already locked. */
      if (!preg_match("/^[^\}]*+\}!/", $pwd)) {
        if ($mode == 'UNLOCK') {
          return TRUE;
        }
      } elseif ($mode == 'LOCK') {
        return TRUE;
      }

      // (Un)lock the samba account
      $modify = lock_samba_account($mode, $attrs);

      // (Un)lock SSH keys
      lock_ssh_account($mode, $attrs, $modify);

      // Call pre hooks
      $userClass = new user($dn);
      $errors = $userClass->callHook('PRE'.$mode, array(), $ret);
      if (!empty($errors)) {
        msg_dialog::displayChecks($errors);
        return FALSE;
      }

      // (Un)lock the account by modifying the password hash.
      if ($mode == 'LOCK') {
        /* Lock entry */
        $pwd = preg_replace("/(^[^\}]+\})(.*$)/",   "\\1!\\2",  $pwd);
      } else {
        /* Unlock entry */
        $pwd = preg_replace("/(^[^\}]+\})!(.*$)/",  "\\1\\2",   $pwd);
      }
      $modify['userPassword'] = $pwd;
      $ldap->cd($dn);
      $ldap->modify($modify);

      // Call the password post-lock hook, if defined.
      if ($ldap->success()) {
        $userClass = new user($dn);
        $errors = $userClass->callHook('POST'.$mode, array(), $ret);
        if (!empty($errors)) {
          msg_dialog::displayChecks($errors);
        }
      } else {
        msg_dialog::display(_('LDAP error'), msgPool::ldaperror($ldap->get_error(), $dn, LDAP_MOD), LDAP_ERROR);
      }
      return $ldap->success();
    }
    return FALSE;
  }


  /*!
   * \brief This function returns all loaded classes for password encryption
   */
  static function get_available_methods()
  {
    global $class_mapping;
    $ret  = FALSE;
    $i    = 0;

    /* Only */
    if (!session::is_set("passwordMethod::get_available_methods")) {
      foreach (array_keys($class_mapping) as $class) {
        if (preg_match('/passwordMethod/i', $class) && !preg_match("/^passwordMethod$/i", $class)) {
          $test = new $class("");
          if ($test->is_available()) {
            $plugs = $test->get_hash_name();
            if (!is_array($plugs)) {
              $plugs = array($plugs);
            }

            foreach ($plugs as $plugname) {
              $cfg = $test->is_configurable();

              $ret['name'][$i]            = $plugname;
              $ret['class'][$i]           = $class;
              $ret['is_configurable'][$i] = $cfg;
              $ret['object'][$i]          = $test;
              $ret['desc'][$i]            = $test->get_description();
              $ret[$i]['name']            = $plugname;
              $ret[$i]['class']           = $class;
              $ret[$i]['object']          = $test;
              $ret[$i]['is_configurable'] = $cfg;
              $ret[$i]['desc']            = $test->get_description();
              $ret[$plugname]             = $class;
              $i++;
            }
          }
        }
      }
      session::set("passwordMethod::get_available_methods", $ret);
    }
    return session::get("passwordMethod::get_available_methods");
  }

  /*!
   * \brief Get desciption
   */
  function get_description()
  {
    return "";
  }

  /*!
   * \brief Method to check if a password matches a hash
   */
  function checkPassword($pwd, $hash)
  {
    return ($hash == $this->generate_hash($pwd));
  }


  /*!
   * \brief Return true if this password method provides a configuration dialog
   */
  function is_configurable()
  {
    return FALSE;
  }

  /*!
   * \brief Provide a subdialog to configure a password method
   */
  function configure()
  {
    return "";
  }


  /*!
   * \brief Save information to LDAP
   *
   * \param string $dn The DN
   */
  function save($dn)
  {
  }


  /*!
   * \brief Try to find out if it's our hash...
   *
   * \param string $password_hash
   *
   * \param string $dn The DN
   */
  static function get_method($password_hash, $dn = "")
  {
    $methods = passwordMethod::get_available_methods();

    foreach ($methods['class'] as $class) {
      $method = $class::_extract_method($class, $password_hash);
      if ($method != "") {
        $test = new $class($dn);
        $test->set_hash($method);
        return $test;
      }
    }

    $method = new passwordMethodClear($dn);
    $method->set_hash('clear');
    return $method;
  }

  /*!
   * \brief Extract a method
   *
   * \param string $classname The password method class name
   *
   * \param string $password_hash
   */
  static function _extract_method($classname, $password_hash)
  {
    $hash = $classname::get_hash_name();
    if (preg_match("/^\{$hash\}/i", $password_hash)) {
      return $hash;
    }

    return "";
  }

  /*!
   * \brief Make a hash
   *
   * \param string $password The password
   *
   * \param string $hash
   */
  static function make_hash($password, $hash)
  {
    $methods  = passwordMethod::get_available_methods();
    $tmp      = new $methods[$hash]();
    $tmp->set_hash($hash);
    return $tmp->generate_hash($password);
  }

  /*!
   * \brief Set a hash
   *
   * \param string $hash
   */
  function set_hash($hash)
  {
    $this->hash = $hash;
  }


  /*!
   * \brief Get a hash
   */
  function get_hash()
  {
    return $this->hash;
  }

  /*!
   * \brief Test for problematic unicode caracters in password
   *  This can be activated with the keyword strictPasswordRules in the
   *  fusiondirectory.conf
   *
   * \param string $password The password
   */
  static function is_harmless($password)
  {
    global $config;
    if ($config->get_cfg_value("strictPasswordRules") == "TRUE") {
      // Do we have UTF8 characters in the password?
      return ($password == utf8_decode($password));
    }

    return TRUE;
  }
}
?>
