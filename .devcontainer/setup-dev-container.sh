#!/bin/bash
set -e

echo "--- Installing C++ tools ---"

export DEBIAN_FRONTEND=noninteractive

sudo apt-get update

sudo apt-get install -y --no-install-recommends \
    clang \
    clangd \
    lldb \
    lld \
    llvm \
    clang-format \
    clang-tidy \
    clang-tools \
    cmake \
    ninja-build \
    cppcheck \
    ccache \
    doxygen \
    graphviz \
    valgrind \
    lcov \
    just \
    curl

echo "--- Installing Rust (Cargo, Rustfmt y Clippy) ---"

REAL_USER=${SUDO_USER:-$(whoami)}
REAL_HOME=$(eval echo ~$REAL_USER)
echo "Instalando Rust para el usuario: $REAL_USER en $REAL_HOME"

sudo -u "$REAL_USER" HOME="$REAL_HOME" curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sudo -u "$REAL_USER" HOME="$REAL_HOME" sh -s -- -y

echo 'source "$HOME/.cargo/env"' >> $REAL_HOME/.bashrc

echo "--- Installation completed successfully! ---"
