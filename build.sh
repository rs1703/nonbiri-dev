#!/bin/sh
cmake -H. -Bbuild/linux -DCMAKE_TOOLCHAIN_FILE=${PWD}/cmake/toolchain.cmake
cmake --build build/linux --config Release
