#!/usr/bin/env bash

#set -euxop

ROOT_DIR=/local

pushd ${ROOT_DIR}
  # prep: purge all build files
  # TODO: this should obviously be `clean`, but don't know how to do that
  rm -rf CMakeCache.txt CMakeFiles *.cmake Makefile
  # prep: delete the binary, just to be safe
  rm -rf RaveCylinder

  # cmake, first run: generates all the meta/build files
  cmake .
  # cmake, second run: actually generate the binary
  cmake --build .
popd
