#!/bin/bash
#./build-all.sh -l -x86 -d -g
cmake --workflow --preset wf-linux-x64-debug-clang

# No funciona:

# Linux armv7 GCC       - (is not able to compile a simple test program)

# Linux x86 Clang       - (is not able to compile a simple test program)
# Linux arm64 Clang     - (is not able to compile a simple test program)
# Linux armv7 Clang     - (is not able to compile a simple test program)

# Windows x64 GCC       - (is not able to compile a simple test program)
# Windows x86 GCC       - (is not able to compile a simple test program)
# Windows arm64 GCC     - (is not able to compile a simple test program)
# Windows armv7 GCC     - (is not able to compile a simple test program)

# Windows x64 Clang     - (is not able to compile a simple test program)
# Windows x86 Clang     - (is not able to compile a simple test program)
# Windows arm64 Clang   - (is not able to compile a simple test program)
# Windows armv7 Clang   - (is not able to compile a simple test program)