#!/bin/bash
set -e

if [ ! -d "include" ]; then
	sh update_api.sh
fi

rm -rf build
mkdir build
cd build
cmake ..
cmake --build . --config Release

mkdir dlls
cp maps/pizza_ya_san/*.so dlls/
