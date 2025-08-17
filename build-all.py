#!/usr/bin/env python3
"""
build-all.py

Ejecuta el workflow de CMake para compilar el proyecto para todos los targets disponibles.
"""

from base import run_command, clear_cache

def main():
    run_command("cmake --workflow --preset wf-linux-x64-debug-clang")
    #run_command("cmake --workflow --preset wf-linux-x64-debug-gcc")
    #run_command("cmake --workflow --preset wf-linux-x64-release-clang")
    #run_command("cmake --workflow --preset wf-linux-x64-release-gcc")
    #run_command("cmake --workflow --preset wf-linux-x86-debug-clang")
    #run_command("cmake --workflow --preset wf-linux-x86-debug-gcc")
    #run_command("cmake --workflow --preset wf-linux-x86-release-clang")
    #run_command("cmake --workflow --preset wf-linux-x86-release-gcc")
    #run_command("cmake --workflow --preset wf-linux-arm64-debug-clang")
    #run_command("cmake --workflow --preset wf-linux-arm64-debug-gcc")
    #run_command("cmake --workflow --preset wf-linux-arm64-release-clang")
    #run_command("cmake --workflow --preset wf-linux-arm64-release-gcc")
    #run_command("cmake --workflow --preset wf-linux-armv7-debug-clang")
    #run_command("cmake --workflow --preset wf-linux-armv7-debug-gcc")
    #run_command("cmake --workflow --preset wf-linux-armv7-release-clang")
    #run_command("cmake --workflow --preset wf-linux-armv7-release-gcc")
    #run_command("cmake --workflow --preset wf-windows-x64-debug-clang")
    #run_command("cmake --workflow --preset wf-windows-x64-debug-gcc")
    #run_command("cmake --workflow --preset wf-windows-x64-release-clang")
    #run_command("cmake --workflow --preset wf-windows-x64-release-gcc")
    #run_command("cmake --workflow --preset wf-windows-x86-debug-clang")
    #run_command("cmake --workflow --preset wf-windows-x86-debug-gcc")
    #run_command("cmake --workflow --preset wf-windows-x86-release-clang")
    #run_command("cmake --workflow --preset wf-windows-x86-release-gcc")
    #run_command("cmake --workflow --preset wf-windows-arm64-debug-clang")
    #run_command("cmake --workflow --preset wf-windows-arm64-debug-gcc")
    #run_command("cmake --workflow --preset wf-windows-arm64-release-clang")
    #run_command("cmake --workflow --preset wf-windows-arm64-release-gcc")
    #run_command("cmake --workflow --preset wf-windows-armv7-debug-clang")
    #run_command("cmake --workflow --preset wf-windows-armv7-debug-gcc")
    #run_command("cmake --workflow --preset wf-windows-armv7-release-clang")
    #run_command("cmake --workflow --preset wf-windows-armv7-release-gcc")
    clear_cache()

if __name__ == "__main__":
    main()