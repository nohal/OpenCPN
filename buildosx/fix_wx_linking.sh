#!/bin/bash

WX_VER=`wx-config --version|cut -c 1-3`
WX_PATCH=`wx-config --version|cut -c 5`

if [ "${WX_VER}" == "3.0" ]; then
  WX_VER_SHORT="${WX_VER}"
  WX_VER_LONG="${WX_VER}.0.${WX_PATCH}.0"
else
  WX_VER_SHORT="${WX_VER}"
  WX_VER_LONG="${WX_VER}.${WX_PATCH}.0.0"
fi

BROKEN_DYLIBS=`otool -L /usr/local/lib/libwx_*-${WX_VER_SHORT}.dylib|grep ${WX_VER_LONG}|wc -l`
if [ $BROKEN_DYLIBS -ne 0 ]; then
  echo "Fixing internal dependencies in wxWidgets ${WX_VER}.${WX_PATCH} residing in $1"
else
  echo "There is no need to fix the internal dependencies in wxWidgets libs"
  exit 1
fi
find $1 -name "libwx_*-${WX_VER_LONG}.dylib" -exec install_name_tool -change /usr/local/lib/libwx_baseu-${WX_VER_LONG}.dylib /usr/local/lib/libwx_baseu-${WX_VER_SHORT}.dylib {} \;
find $1 -name "libwx_*-${WX_VER_LONG}.dylib" -exec install_name_tool -change /usr/local/lib/libwx_baseu_xml-${WX_VER_LONG}.dylib /usr/local/lib/libwx_baseu_xml-${WX_VER_SHORT}.dylib {} \;
find $1 -name "libwx_*-${WX_VER_LONG}.dylib" -exec install_name_tool -change /usr/local/lib/libwx_osx_cocoau_core-${WX_VER_LONG}.dylib /usr/local/lib/libwx_osx_cocoau_core-${WX_VER_SHORT}.dylib {} \;
find $1 -name "libwx_*-${WX_VER_LONG}.dylib" -exec install_name_tool -change /usr/local/lib/libwx_osx_cocoau_adv-${WX_VER_LONG}.dylib /usr/local/lib/libwx_osx_cocoau_adv-${WX_VER_SHORT}.dylib {} \;
find $1 -name "libwx_*-${WX_VER_LONG}.dylib" -exec install_name_tool -change /usr/local/lib/libwx_osx_cocoau_html-${WX_VER_LONG}.dylib /usr/local/lib/libwx_osx_cocoau_html-${WX_VER_SHORT}.dylib {} \;

