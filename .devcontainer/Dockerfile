FROM mcr.microsoft.com/devcontainers/base:ubuntu

# Powershell
RUN wget -q https://packages.microsoft.com/config/ubuntu/$(lsb_release -rs)/packages-microsoft-prod.deb && \
    dpkg -i packages-microsoft-prod.deb && \
    rm ./packages-microsoft-prod.deb

RUN sudo apt update && \
    apt install -y \
    powershell \
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
    valgrind
