FROM mcr.microsoft.com/devcontainers/base:ubuntu

# LLVM
RUN apt update && \
    apt install -y curl build-essential lsb-release wget software-properties-common gnupg && \
    wget https://apt.llvm.org/llvm.sh && \
    chmod +x llvm.sh && \
    ./llvm.sh && \
    rm ./llvm.sh && \
    update-alternatives --install /usr/bin/clang clang /usr/bin/clang-20 100 && \
    update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-20 100 && \
    update-alternatives --install /usr/bin/lldb lldb /usr/bin/lldb-20 100 && \
    update-alternatives --install /usr/bin/lld lld /usr/bin/lld-20 100 && \
    update-alternatives --install /usr/bin/clangd clangd /usr/bin/clangd-20 100

# Rust
#RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y --default-toolchain stable
#RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y

# Other
RUN apt install -y \
    clang-format \    
    clang-tidy \
    cmake \
    ninja-build \
    cppcheck \
    valgrind \
    doxygen \
    graphviz \
    ccache
