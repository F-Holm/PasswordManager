#!/bin/bash
#./build-all.sh -w -x64 -d -g
cmake --workflow --preset wf-windows-x64-debug-clang

# No funciona:

# Linux x86 Clang       - (is not able to compile a simple test program)
# Linux arm64 Clang     - (is not able to compile a simple test program)
# Linux armv7 Clang     - (is not able to compile a simple test program)

# Windows x64 GCC       - (No encuentra librerías)
# Windows x86 GCC       - (No encuentra librerías)
# Windows arm64 GCC     - (No encuentra librerías)
# Windows armv7 GCC     - (No encuentra librerías)

# Windows x64 Clang     - (No encuentra librerías)
# Windows x86 Clang     - (No encuentra librerías)
# Windows arm64 Clang   - (No encuentra librerías)
# Windows armv7 Clang   - (No encuentra librerías)