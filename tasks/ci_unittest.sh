#!/bin/sh
set -eux

cmake -G Ninja -D CMAKE_C_COMPILER=clang -D CMAKE_CXX_COMPILER=clang++ -D CMAKE_BUILD_TYPE=Release ../ets2_telemetry_mqtt
cmake --build . --target ets2clienttest
ctest . -V --output-on-failure
find .
