#!/usr/bin/env bash

set -e

set_result() {
  NEW_RESULT=$1
  LAST_RESULT=$2

  if [ $LAST_RESULT -ne 0 ] && [ $NEW_RESULT -eq 0 ]
  then
    NEW_RESULT=$LAST_RESULT
  fi

  echo $NEW_RESULT
}

mkdir build && cd build
if [[ "$TRAVIS_OS_NAME" == "linux" ]] && [[ "$TESTS" != "STATIC" ]];
  then cmake -DCMAKE_BUILD_TYPE=Debug ..
fi
if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
  if [ -z "$TRAVIS_TAG" ]; then
    # CI build, include commit ID
    cmake -DOCPN_CI_BUILD=ON -DOCPN_USE_LIBCPP=ON -DwxWidgets_CONFIG_EXECUTABLE=/tmp/wx312_opencpn50_macos109/bin/wx-config -DwxWidgets_CONFIG_OPTIONS="--prefix=/tmp/wx312_opencpn50_macos109" -DCMAKE_INSTALL_PREFIX=/tmp/opencpn -DCMAKE_OSX_DEPLOYMENT_TARGET=10.9 ..
  else
    # Build from tag, include the version number only
    cmake -DOCPN_CI_BUILD=OFF -DOCPN_USE_LIBCPP=ON -DwxWidgets_CONFIG_EXECUTABLE=/tmp/wx312_opencpn50_macos109/bin/wx-config -DwxWidgets_CONFIG_OPTIONS="--prefix=/tmp/wx312_opencpn50_macos109" -DCMAKE_INSTALL_PREFIX=/tmp/opencpn -DCMAKE_OSX_DEPLOYMENT_TARGET=10.9 ..
  fi
  make -sj2
  mkdir -p /tmp/opencpn/bin/OpenCPN.app/Contents/MacOS
  mkdir -p /tmp/opencpn/bin/OpenCPN.app/Contents/SharedSupport/plugins
  chmod 644 /usr/local/lib/lib*.dylib
  make install
  make install # Dunno why the second is needed but it is, otherwise plugin data is not included in the bundle
  make create-dmg
fi
if [[ "$TRAVIS_OS_NAME" == "linux" ]] && [[ "$TESTS" != "STATIC" ]]; then
  make -sj2
  make package
fi
if [[ "$TRAVIS_OS_NAME" == "linux" ]] && [[ "$TESTS" == "STATIC" ]]; then
  cd ..
  RESULT=0
  git status
  git branch -a
  git rebase origin/$TRAVIS_BRANCH || git rebase --abort
  RESULT=$(set_result $? $RESULT)

  ./ci_check_whitespace.sh master
  RESULT=$(set_result $? $RESULT)

  exit $RESULT
fi
