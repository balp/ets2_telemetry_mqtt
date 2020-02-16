FROM alpine:3.11.3
RUN apk add --no-cache \
        cmake \
        clang \
        compiler-rt \
        compiler-rt-static \
        git \
        ninja \
        binutils \
        build-base \
        openssh \
        openssl-dev \
        mosquitto-dev \
        graphviz \
        python3 \
        python3-dev \
        doxygen \
      && pip3 install --no-cache-dir bumpversion \
      && rm -rf /var/cache/* \
      && rm -rf /root/.cache/*

