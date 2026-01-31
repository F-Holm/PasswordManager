FROM mcr.microsoft.com/devcontainers/base:ubuntu

RUN apt update && \
    apt install -y \
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
    pkg-config \
    python3 \
    python3-pip \
    python3-venv \
    pipx

RUN pipx ensurepath && \
    pipx install conan
