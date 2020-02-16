#!/bin/sh
set -eux

pwd
ls -la ..
ls -la ../html_docs
ls -la ../ets2_telemetry_mqtt


# (cd ../ets2_telemetry_mqtt ; git describe --tags --always) > name
echo "Test v0.0.0" > name
echo v0.0.0 > tag
# (cd ../ets2_telemetry_mqtt ; git log --pretty=format:"%h - %an, %ar : %s" --since=2.weeks) > body
echo "Release info goes here!!!" > body
mkdir -p files
tar c -C ../html_docs -j -f files/docs.tar.bz2 .
cp ../build_alpine_clang/libets2telemery.so files/

find .