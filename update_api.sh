#!/bin/bash
set -e

rm -rf include
rm -rf lib
mkdir lib
mkdir include

git submodule update --recursive --remote

cd SevenKewp

python3 plugin_api.py ../include
rm -rf build
mkdir build
cd build
cmake -DBUILD_SERVER_ONLY=ON ..
cmake --build . --config Release
cp dlls/sevenkewp.so ../../lib/
