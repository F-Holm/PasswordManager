FROM mcr.microsoft.com/devcontainers/base:ubuntu

# PowerShell
RUN wget -q https://packages.microsoft.com/config/ubuntu/$(lsb_release -rs)/packages-microsoft-prod.deb && \
    dpkg -i packages-microsoft-prod.deb && \
    rm ./packages-microsoft-prod.deb

# Dependencias
RUN apt update && apt install -y \
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
    libssl-dev \
    libwxgtk3.2-dev \
    libgtk-3-dev \
    pkg-config
