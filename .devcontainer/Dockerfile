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

COPY ./CLANG-CROSS-COMPILING-WRAPPERS.sh ./CLANG-CROSS-COMPILING-WRAPPERS.sh
COPY ./UPDATE-LLVM-MINGW.sh ./UPDATE-LLVM-MINGW.sh
COPY ./GET-NAME-LLVM-MINGW-RELEASE.py ./GET-NAME-LLVM-MINGW-RELEASE.py

RUN ./CLANG-CROSS-COMPILING-WRAPPERS.sh
RUN ./UPDATE-LLVM-MINGW.sh

RUN rm ./CLANG-CROSS-COMPILING-WRAPPERS.sh
RUN rm ./UPDATE-LLVM-MINGW.sh