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
    cppcheck \
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
    mingw-w64
    #pkg-config \
    #libgtk-3-dev \
    #libcurl4-openssl-dev \
    #libsecret-1-dev \
    #libwebkit2gtk-4.0-dev \
    #libsoup2.4-dev \
    #libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev \
    #libsdl2-dev \
    #libnotify-dev

# No funcionan
# Scripts
#RUN git clone https://github.com/F-Holm/Paquetes-Ubuntu.git

#RUN ./Paquetes-Ubuntu/CLANG-CROSS-COMPILING-WRAPPERS.sh
#RUN ./Paquetes-Ubuntu/UPDATE-LLVM-MINGW.sh

#RUN rm -rf ./Paquetes-Ubuntu
