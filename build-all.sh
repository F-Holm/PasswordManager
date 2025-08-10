#!/bin/bash

# Borrar /build y resetear openssl-src
./clean.sh
ccache -C

# Build workflows (los comentados todavía no están testeados pero ya están configurados)
#cmake --workflow --preset wf-windows-x64-release-clang && ccache -C
#cmake --workflow --preset wf-windows-x64-release-gcc && ccache -C
#cmake --workflow --preset wf-windows-x64-debug-clang && ccache -C
#cmake --workflow --preset wf-windows-x64-debug-gcc && ccache -C
#cmake --workflow --preset wf-windows-x86-release-clang && ccache -C
#cmake --workflow --preset wf-windows-x86-release-gcc && ccache -C
#cmake --workflow --preset wf-windows-x86-debug-clang && ccache -C
#cmake --workflow --preset wf-windows-x86-debug-gcc && ccache -C
#cmake --workflow --preset wf-windows-arm64-release-clang && ccache -C
#cmake --workflow --preset wf-windows-arm64-release-gcc && ccache -C
#cmake --workflow --preset wf-windows-arm64-debug-clang && ccache -C
#cmake --workflow --preset wf-windows-arm64-debug-gcc && ccache -C
#cmake --workflow --preset wf-windows-arm-release-clang && ccache -C
#cmake --workflow --preset wf-windows-arm-release-gcc && ccache -C
#cmake --workflow --preset wf-windows-arm-debug-clang && ccache -C
#cmake --workflow --preset wf-windows-arm-debug-gcc && ccache -C
cmake --workflow --preset wf-linux-x64-release-clang && ccache -C
cmake --workflow --preset wf-linux-x64-release-gcc && ccache -C
cmake --workflow --preset wf-linux-x64-debug-clang && ccache -C
cmake --workflow --preset wf-linux-x64-debug-gcc && ccache -C
#cmake --workflow --preset wf-linux-x86-release-clang && ccache -C
#cmake --workflow --preset wf-linux-x86-release-gcc && ccache -C
#cmake --workflow --preset wf-linux-x86-debug-clang && ccache -C
#cmake --workflow --preset wf-linux-x86-debug-gcc && ccache -C
#cmake --workflow --preset wf-linux-arm64-release-clang && ccache -C
#cmake --workflow --preset wf-linux-arm64-release-gcc && ccache -C
#cmake --workflow --preset wf-linux-arm64-debug-clang && ccache -C
#cmake --workflow --preset wf-linux-arm64-debug-gcc && ccache -C
#cmake --workflow --preset wf-linux-arm-release-clang && ccache -C
#cmake --workflow --preset wf-linux-arm-release-gcc && ccache -C
#cmake --workflow --preset wf-linux-arm-debug-clang && ccache -C
#cmake --workflow --preset wf-linux-arm-debug-gcc && ccache -C