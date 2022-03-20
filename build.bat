cmake -H. -Bbuild/windows -G "Visual Studio 16 2019" -DCMAKE_TOOLCHAIN_FILE=%cd%\cmake\toolchain.cmake
cmake --build build/windows --config Release