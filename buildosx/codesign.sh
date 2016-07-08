#!/bin/bash
if [ $# -eq 3 ]; then
  if [ "$1" == "BUNDLE" ] && [ "$2" != "" ]; then
    echo "Going to codesign the application bundle using identity \"$2\". You will be prompted for your user credentials many times (for each of the files being signed)."
  elif [ "$1" == "DMG" ] && [ "$2" != "" ]; then
    echo "Going to codesign the DMG image."
  else
    echo "Not signing anything, provide a valid identity by setting -DCODESIGN_IDENTITY=\"Some identity from your keychain\" while running cmake."
    exit 1
  fi
else
  echo "Not going to sign anything, provide a valid identity by setting -DCODESIGN_IDENTITY=\"Some identity from your keychain\" while running cmake."
  exit 1
fi
if [ "$1" == "BUNDLE" ]; then
  codesign --deep --force -s "$2" -i org.opencpn.opencpn $3
else
  codesign -s "$2" "$3"
fi
