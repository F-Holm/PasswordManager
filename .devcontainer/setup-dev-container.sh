#!/bin/bash
set -e

echo "--- Actualizando paquetes e instalando herramientas de C++ ---"

echo "--- Actualizando paquetes e instalando herramientas de C++ ---"

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

echo "--- Instalando Rust (Cargo, Rustfmt y Clippy) ---"

curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y

source "$HOME/.cargo/env"

echo "Versiones instaladas:"
cargo --version
rustfmt --version
cargo clippy --version

echo "--- ¡Instalación completada con éxito! ---"
