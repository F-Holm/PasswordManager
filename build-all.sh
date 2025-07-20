#!/bin/bash
cmake --workflow --preset wf-windows-x64-release
cmake --workflow --preset wf-windows-x64-debug
cmake --workflow --preset wf-windows-x86-release
cmake --workflow --preset wf-windows-x86-debug
#cmake --workflow --preset wf-windows-arm64-release
#cmake --workflow --preset wf-windows-arm64-debug
#cmake --workflow --preset wf-windows-arm-release
#cmake --workflow --preset wf-windows-arm-debug
cmake --workflow --preset wf-linux-x64-release
cmake --workflow --preset wf-linux-x64-debug
cmake --workflow --preset wf-linux-x86-release
cmake --workflow --preset wf-linux-x86-debug
cmake --workflow --preset wf-linux-arm64-release
cmake --workflow --preset wf-linux-arm64-debug
cmake --workflow --preset wf-linux-arm-release
cmake --workflow --preset wf-linux-arm-debug