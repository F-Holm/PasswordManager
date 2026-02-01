FROM mcr.microsoft.com/devcontainers/base:ubuntu

RUN wget -q https://packages.microsoft.com/config/ubuntu/$(lsb_release -rs)/packages-microsoft-prod.deb && \
    dpkg -i packages-microsoft-prod.deb && \
    rm ./packages-microsoft-prod.deb

RUN apt update && \
    apt install -y \
    powershell \
    cmake \
    ninja-build \
    clang \
    clang-format \
    clang-tidy \
    clang-tools \
    llvm \
    cppcheck \
    valgrind \
    doxygen \
    graphviz \
    ccache \
    libgtk-3-dev \
    libpng-dev \
    pkg-config \
    gcc \
    g++ \
    gcc-i686-linux-gnu \
    g++-i686-linux-gnu \
    gcc-arm-linux-gnueabihf \
    g++-arm-linux-gnueabihf \
    gcc-aarch64-linux-gnu \
    g++-aarch64-linux-gnu \
    perl

RUN wget https://github.com/mstorsjo/llvm-mingw/releases/download/20251216/llvm-mingw-20251216-ucrt-ubuntu-22.04-x86_64.tar.xz && \
    tar -xf llvm-mingw-*.tar.xz && \
    rm llvm-mingw-*.tar.xz && \
    mv llvm-mingw* /opt/llvm-mingw
