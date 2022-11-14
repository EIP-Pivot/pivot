#!/usr/bin/env bash

set -e

build_dir=${1:-build.appimage}
echo Build directory $build_dir

app_dir=$build_dir/AppDir

cmake -B $build_dir -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build $build_dir --parallel
cmake --install $build_dir --prefix $app_dir