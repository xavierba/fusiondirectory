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

class locality extends department
{
  static $namingAttr  = "l";
  var $objectclasses  = array("top", "gosaDepartment");
  var $structuralOC   = array("locality");

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Locality'),
      'plDescription' => _('Locality'),
      'plObjectType'  => array('locality' => array(
        'aclCategory' => 'department',
        'name'        => _('Locality'),
        'filter'      => '(&(!(objectClass=dcObject))(objectClass=locality)(objectClass=gosaDepartment))',
        'ou'          => '',
        'icon'        => 'geticon.php?context=types&icon=locality&size=16',
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
    $attributesInfo = parent::getDepartmentAttributesInfo(_('locality'));
    unset($attributesInfo['location']);
    unset($attributesInfo['properties']['attrs'][3]); // category
    return $attributesInfo;
  }
}
?>
