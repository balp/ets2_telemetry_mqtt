#!/bin/bash
set -euxo pipefail
IMAGE=balp/ets2_telemetry_mqtt-build-tools:latest

mkdir -p build_ubuntu_clang
docker run -v $(pwd):/src -w /src/build_ubuntu_clang/ ${IMAGE} cmake -G Ninja -D CMAKE_C_COMPILER=clang -D CMAKE_CXX_COMPILER=clang++ -D CMAKE_BUILD_TYPE=Debug -D UBUNTU32=True -D USE_HIQSIM=True ..
docker run -v $(pwd):/src -w /src/build_ubuntu_clang/ ${IMAGE} cmake --build .
docker run -v $(pwd):/src -w /src/build_ubuntu_clang/ ${IMAGE} ctest .
