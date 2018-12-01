#!/bin/bash
set -euxo pipefail

mkdir -p build
cd build
cmake -G Ninja -D CMAKE_C_COMPILER=clang -D CMAKE_CXX_COMPILER=clang++ -D CMAKE_BUILD_TYPE=Debug ..
cmake --build . --target ets2telemery
cmake --build . --target ets2clienttest -- -v
ctest .

cp libets2telemery.so ~/.local/share/Steam/steamapps/common/Euro\ Truck\ Simulator\ 2/bin/linux_x64/plugins
