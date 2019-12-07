#!/bin/bash
set -euxo pipefail

mkdir -p build
cd build
cmake -G Ninja -D CMAKE_C_COMPILER=clang -D CMAKE_CXX_COMPILER=clang++ -D CMAKE_BUILD_TYPE=Debug ..
cmake --build . --target ets2telemery
VERBOSE=True cmake --build . --target ets2clienttest -- -v
ctest .

mkdir -p ~/.local/share/Steam/steamapps/common/Euro\ Truck\ Simulator\ 2/bin/linux_x64/plugins
cp libets2telemery.so ~/.local/share/Steam/steamapps/common/Euro\ Truck\ Simulator\ 2/bin/linux_x64/plugins
mkdir -p ~/.local/share/Steam/steamapps/common/American\ Truck\ Simulator/bin/linux_x64/plugins
cp libets2telemery.so ~/.local/share/Steam/steamapps/common/American\ Truck\ Simulator/bin/linux_x64/plugins
