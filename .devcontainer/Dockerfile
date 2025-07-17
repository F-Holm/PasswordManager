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
    g++-aarch64-linux-gnu \
    wget

# Install LLVM-Mingw
#RUN wget https://github.com/mstorsjo/llvm-mingw/releases/download/20250709/llvm-mingw-20250709-ucrt-ubuntu-22.04-x86_64.tar.xz -P /opt/
#RUN tar -xf ./deps/llvm-mingw-*.tar.xz -C "$(dirname ./deps/llvm-mingw/llvm-mingw-*.tar.xz)"
#RUN rm ./deps/llvm-mingw*.tar.xz

# Crear directorio para deps
RUN mkdir -p /opt/llvm-mingw

# Descargar y descomprimir LLVM-Mingw
RUN wget https://github.com/mstorsjo/llvm-mingw/releases/download/20250709/llvm-mingw-20250709-ucrt-ubuntu-22.04-x86_64.tar.xz -P /tmp && \
    tar -xf /tmp/llvm-mingw-*.tar.xz -C /opt/llvm-mingw --strip-components=1 && \
    rm /tmp/llvm-mingw-*.tar.xz

# Agregar al PATH para que clang lo detecte
ENV PATH="/opt/llvm-mingw/bin:${PATH}"