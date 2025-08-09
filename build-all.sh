#!/bin/bash

# Borrar /build y resetear openssl-src
./clean.sh
ccache -C

# Build workflows (los comentados todavía no están testeados pero ya están configurados)
#cmake --workflow --preset wf-windows-x64-release && ccache -C
#cmake --workflow --preset wf-windows-x64-debug && ccache -C
#cmake --workflow --preset wf-windows-x86-release && ccache -C
#cmake --workflow --preset wf-windows-x86-debug && ccache -C
#cmake --workflow --preset wf-windows-arm64-release && ccache -C
#cmake --workflow --preset wf-windows-arm64-debug && ccache -C
#cmake --workflow --preset wf-windows-arm-release && ccache -C
#cmake --workflow --preset wf-windows-arm-debug && ccache -C
cmake --workflow --preset wf-linux-x64-release && ccache -C
cmake --workflow --preset wf-linux-x64-debug && ccache -C
#cmake --workflow --preset wf-linux-x86-release && ccache -C
#cmake --workflow --preset wf-linux-x86-debug && ccache -C
#cmake --workflow --preset wf-linux-arm64-release && ccache -C
#cmake --workflow --preset wf-linux-arm64-debug && ccache -C
#cmake --workflow --preset wf-linux-arm-release && ccache -C
#cmake --workflow --preset wf-linux-arm-debug && ccache -C