#!/bin/sh
set -eux

pwd
ls -la ..
ls -la ../html_docs
ls -la ../ets2_telemetry_mqtt

(cd ../ets2_telemetry_mqtt ; /script/bump --input "$(git describe --tags --always)" "$(git log -1 --pretty=%B)" ) > name
(cd ../ets2_telemetry_mqtt ; /script/bump --input "$(git describe --tags --always)" "$(git log -1 --pretty=%B)" ) > tag
(cd ../ets2_telemetry_mqtt ; git log --pretty=format:"%h - %an, %ar : %s" --since=2.weeks) > body
mkdir -p files
tar c -C ../html_docs -j -f files/docs.tar.bz2 .
cp ../build_alpine_clang/libets2telemery.so files/

/script/bump --input "$(git describe --tags --always)" --extraxt

find .