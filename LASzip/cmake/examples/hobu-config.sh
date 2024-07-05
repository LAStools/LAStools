#!/bin/bash

GENERATOR="Unix Makefiles"
BUILD_TYPE="Debug"

rm -rf build
mkdir build
cd build
cmake -G "$GENERATOR" \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    ..
