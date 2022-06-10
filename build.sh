#!/bin/sh
cmake -H. -DCMAKE_BUILD_TYPE=Release -Bbuild/linux -DCMAKE_TOOLCHAIN_FILE=${PWD}/cmake/toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build/linux --config Release
