#!/bin/bash
set -euxo pipefail
IMAGE_NAME=balp/ets2_telemetry_mqtt-build-tools:latest
docker build -t $IMAGE_NAME build-tools
# docker push $IMAGE_NAME
