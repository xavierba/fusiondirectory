<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013-2018  FusionDirectory

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

class domain extends department
{
  static $namingAttr  = "dc";
  var $objectclasses  = array("top", "domain", "gosaDepartment");
  var $structuralOC   = array("domain");

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Domain'),
      'plDescription' => _('Domain'),
      'plObjectType'  => array('domain' => array(
        'aclCategory' => 'department',
        'name'        => _('Domain'),
        'filter'      => '(&(objectClass=domain)(objectClass=gosaDepartment))',
        'ou'          => '',
        'icon'        => 'geticon.php?context=types&icon=domain&size=16',
        'tabClass'    => 'deptabs',
        'mainAttr'    => static::$namingAttr,
      )),
      'plForeignKeys'  => array(
        'manager' => array('user','dn','manager=%oldvalue%','*')
      ),

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    $attributesInfo = parent::getDepartmentAttributesInfo(_('domain'));
    unset($attributesInfo['location']);
    unset($attributesInfo['properties']['attrs'][3]); // category
    return $attributesInfo;
  }
}
?>
