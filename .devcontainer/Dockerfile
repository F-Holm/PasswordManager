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
    lcov \
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

# Scripts
RUN git clone https://github.com/F-Holm/Paquetes-Ubuntu.git

RUN ./Paquetes-Ubuntu/CLANG-CROSS-COMPILING-WRAPPERS.sh
RUN ./Paquetes-Ubuntu/UPDATE-LLVM-MINGW.sh

RUN rm -rf ./Paquetes-Ubuntu
