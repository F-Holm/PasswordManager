#!/bin/bash

# Hace que el script se detenga ante cualquier error
set -e

# Borrar /build y resetear openssl-src
./clean.sh
ccache -C

# Build workflows (los comentados todavía no están testeados pero ya están configurados)
#cmake --workflow --preset wf-windows-x64-debug-clang && ccache -C
#cmake --workflow --preset wf-windows-x64-debug-gcc && ccache -C
#cmake --workflow --preset wf-windows-x64-release-clang && ccache -C
#cmake --workflow --preset wf-windows-x64-release-gcc && ccache -C
#cmake --workflow --preset wf-windows-x86-debug-clang && ccache -C
#cmake --workflow --preset wf-windows-x86-debug-gcc && ccache -C
#cmake --workflow --preset wf-windows-x86-release-clang && ccache -C
#cmake --workflow --preset wf-windows-x86-release-gcc && ccache -C
#cmake --workflow --preset wf-windows-arm64-debug-clang && ccache -C
#cmake --workflow --preset wf-windows-arm64-debug-gcc && ccache -C
#cmake --workflow --preset wf-windows-arm64-release-clang && ccache -C
#cmake --workflow --preset wf-windows-arm64-release-gcc && ccache -C
#cmake --workflow --preset wf-windows-armv7-debug-clang && ccache -C
#cmake --workflow --preset wf-windows-armv7-debug-gcc && ccache -C
#cmake --workflow --preset wf-windows-armv7-release-clang && ccache -C
#cmake --workflow --preset wf-windows-armv7-release-gcc && ccache -C
cmake --workflow --preset wf-linux-x64-debug-clang && ccache -C
cmake --workflow --preset wf-linux-x64-debug-gcc && ccache -C
cmake --workflow --preset wf-linux-x64-release-clang && ccache -C
cmake --workflow --preset wf-linux-x64-release-gcc && ccache -C
#cmake --workflow --preset wf-linux-x86-debug-clang && ccache -C
#cmake --workflow --preset wf-linux-x86-debug-gcc && ccache -C
#cmake --workflow --preset wf-linux-x86-release-clang && ccache -C
#cmake --workflow --preset wf-linux-x86-release-gcc && ccache -C
#cmake --workflow --preset wf-linux-arm64-debug-clang && ccache -C
#cmake --workflow --preset wf-linux-arm64-debug-gcc && ccache -C
#cmake --workflow --preset wf-linux-arm64-release-clang && ccache -C
#cmake --workflow --preset wf-linux-arm64-release-gcc && ccache -C
#cmake --workflow --preset wf-linux-armv7-debug-clang && ccache -C
#cmake --workflow --preset wf-linux-armv7-debug-gcc && ccache -C
#cmake --workflow --preset wf-linux-armv7-release-clang && ccache -C
#cmake --workflow --preset wf-linux-armv7-release-gcc && ccache -C