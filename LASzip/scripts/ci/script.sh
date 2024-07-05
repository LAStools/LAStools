#!/bin/bash -e
# Builds and tests LASzip

source ./scripts/ci/common.sh
mkdir -p _build || exit 1
cd _build || exit 1

cmake ..

make

#./bin/laszippertest
