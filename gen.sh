#!/bin/bash
rm -rf build
mkdir build
pushd build
cmake ..
mv compile_commands.json ..
popd
