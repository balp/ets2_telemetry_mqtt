#!/bin/sh
set -eux

pwd
ls -la ..
ls -la ../html_docs
ls -la ../ets2_telemetry_mqtt

cd ../ets2_telemetry_mqtt
git_describe=$(git describe --tags --always)
git_log=$(git log -1 --pretty=%B)
semver=$(/script/bump --input "$git_describe" "$git_log")
oldver=$(/script/bump --input "$git_describe" --format M --extract)
cd ../version


echo "Release $semver" > name
echo "v$semver" > tag
(cd ../ets2_telemetry_mqtt ; git log --pretty=format:"%h - %an, %ar : %s" "v$oldver.0.0...") > body
mkdir -p files
tar c -C ../html_docs -j -f files/docs.tar.bz2 .
cp ../build_alpine_clang/libets2telemery.so files/


find .