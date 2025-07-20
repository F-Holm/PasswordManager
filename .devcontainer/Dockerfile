FROM mcr.microsoft.com/devcontainers/base:ubuntu

RUN apt update && apt install -y \
    build-essential \
    cmake \
    ninja-build \
    make \
    perl \
    clang \
    clang-format \
    clang-tidy \
    clang-tools \
    llvm \
    valgrind \
    doxygen \
    graphviz \
    ccache \
    gcc \
    g++ \
    gcc-i686-linux-gnu \
    g++-i686-linux-gnu \
    gcc-arm-linux-gnueabihf \
    g++-arm-linux-gnueabihf \
    gcc-aarch64-linux-gnu \
    g++-aarch64-linux-gnu

# Install LLVM-Mingw
RUN mkdir -p /opt/llvm-mingw
RUN wget https://github.com/mstorsjo/llvm-mingw/releases/download/20250709/llvm-mingw-20250709-ucrt-ubuntu-22.04-x86_64.tar.xz -P /tmp && \
    tar -xf /tmp/llvm-mingw-*.tar.xz -C /opt/llvm-mingw --strip-components=1 && \
    rm /tmp/llvm-mingw-*.tar.xz
ENV PATH="/opt/llvm-mingw/bin:${PATH}"

# Crear wrappers Clang personalizados para cross-compiling en linux
RUN set -eux; \
    mkdir -p /usr/local/bin; \
    echo '#!/bin/sh' > /usr/local/bin/i686-linux-gnu-clang; \
    echo 'exec clang --target=i686-linux-gnu --sysroot=/usr/i686-linux-gnu "$@"' >> /usr/local/bin/i686-linux-gnu-clang; \
    chmod +x /usr/local/bin/i686-linux-gnu-clang; \
    \
    echo '#!/bin/sh' > /usr/local/bin/i686-linux-gnu-clang++; \
    echo 'exec clang++ --target=i686-linux-gnu --sysroot=/usr/i686-linux-gnu "$@"' >> /usr/local/bin/i686-linux-gnu-clang++; \
    chmod +x /usr/local/bin/i686-linux-gnu-clang++; \
    \
    echo '#!/bin/sh' > /usr/local/bin/arm-linux-gnueabihf-clang; \
    echo 'exec clang --target=arm-linux-gnueabihf --sysroot=/usr/arm-linux-gnueabihf "$@"' >> /usr/local/bin/arm-linux-gnueabihf-clang; \
    chmod +x /usr/local/bin/arm-linux-gnueabihf-clang; \
    \
    echo '#!/bin/sh' > /usr/local/bin/arm-linux-gnueabihf-clang++; \
    echo 'exec clang++ --target=arm-linux-gnueabihf --sysroot=/usr/arm-linux-gnueabihf "$@"' >> /usr/local/bin/arm-linux-gnueabihf-clang++; \
    chmod +x /usr/local/bin/arm-linux-gnueabihf-clang++; \
    \
    echo '#!/bin/sh' > /usr/local/bin/aarch64-linux-gnu-clang; \
    echo 'exec clang --target=aarch64-linux-gnu --sysroot=/usr/aarch64-linux-gnu "$@"' >> /usr/local/bin/aarch64-linux-gnu-clang; \
    chmod +x /usr/local/bin/aarch64-linux-gnu-clang; \
    \
    echo '#!/bin/sh' > /usr/local/bin/aarch64-linux-gnu-clang++; \
    echo 'exec clang++ --target=aarch64-linux-gnu --sysroot=/usr/aarch64-linux-gnu "$@"' >> /usr/local/bin/aarch64-linux-gnu-clang++; \
    chmod +x /usr/local/bin/aarch64-linux-gnu-clang++