cmake -H. -Bbuild/windows -G "Visual Studio 16 2019" -DCMAKE_TOOLCHAIN_FILE=%cd%\cmake\toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build/windows --config Release