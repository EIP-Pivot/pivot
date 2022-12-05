#!/usr/bin/env bash

set -e

build_dir=$(realpath ${1:-build.appimage})
echo Build directory $build_dir

app_dir=$(realpath AppDir)

cmake -B $build_dir -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -D CMAKE_CXX_COMPILER=g++-12 -DMIMALLOC_ENABLED=OFF
cmake --build $build_dir --parallel
make -C $build_dir DESTDIR=$app_dir install
appimage-builder --skip-tests